#define WIN32_LEAN_AND_MEAN
#define SECURITY_WIN32

#include <winsock2.h>
#include <windows.h>
#include <schannel.h>
#include <security.h>

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "rictus_tls.h"

typedef struct rictus_schannel_connection {
    CredHandle credentials;
    CtxtHandle context;
    int credentials_valid;
    int context_valid;
} rictus_schannel_connection;

static void set_error(char *error, size_t error_size, const char *message)
{
    if (error != NULL && error_size > 0U) {
        (void)snprintf(error, error_size, "%s", message);
    }
}

static void set_status_error(char *error,
                             size_t error_size,
                             const char *operation,
                             SECURITY_STATUS status)
{
    if (error != NULL && error_size > 0U) {
        (void)snprintf(error,
                       error_size,
                       "%s failed with Schannel status 0x%08lx",
                       operation,
                       (unsigned long)status);
    }
}

static int send_all(SOCKET socket_value, const unsigned char *data, size_t length)
{
    size_t sent = 0U;

    while (sent < length) {
        int chunk = send(socket_value,
                         (const char *)data + sent,
                         (int)(length - sent),
                         0);
        if (chunk <= 0) {
            return 0;
        }
        sent += (size_t)chunk;
    }

    return 1;
}

int rictus_tls_connect(rictus_tls_connection *tls,
                       rictus_net_connection *transport,
                       const char *host,
                       char *error,
                       size_t error_size)
{
    rictus_schannel_connection *native;
    void *handle = NULL;
    SOCKET socket_value;
    SCHANNEL_CRED credentials;
    TimeStamp expiry;
    SECURITY_STATUS status;
    SecBufferDesc output_desc;
    SecBuffer output_buffer;
    SecBufferDesc input_desc;
    SecBuffer input_buffers[2];
    unsigned char input_data[32768];
    size_t input_length = 0U;
    ULONG context_attributes = 0U;
    ULONG request_flags =
        ISC_REQ_SEQUENCE_DETECT |
        ISC_REQ_REPLAY_DETECT |
        ISC_REQ_CONFIDENTIALITY |
        ISC_REQ_EXTENDED_ERROR |
        ISC_REQ_ALLOCATE_MEMORY |
        ISC_REQ_STREAM;

    if (tls == NULL || transport == NULL || host == NULL || host[0] == '\0') {
        set_error(error, error_size, "invalid TLS connection request");
        return 0;
    }

    tls->native = NULL;

    if (!rictus_net_native_handle(transport, &handle) || handle == NULL) {
        set_error(error, error_size, "TLS transport is unavailable");
        return 0;
    }

    socket_value = (SOCKET)(uintptr_t)handle;
    native = (rictus_schannel_connection *)calloc(1U, sizeof(*native));
    if (native == NULL) {
        set_error(error, error_size, "unable to allocate TLS connection");
        return 0;
    }

    memset(&credentials, 0, sizeof(credentials));
    credentials.dwVersion = SCHANNEL_CRED_VERSION;
    credentials.grbitEnabledProtocols = SP_PROT_TLS1_2_CLIENT | SP_PROT_TLS1_3_CLIENT;
    credentials.dwFlags = SCH_CRED_AUTO_CRED_VALIDATION | SCH_CRED_NO_DEFAULT_CREDS;

    status = AcquireCredentialsHandleA(NULL,
                                       UNISP_NAME_A,
                                       SECPKG_CRED_OUTBOUND,
                                       NULL,
                                       &credentials,
                                       NULL,
                                       NULL,
                                       &native->credentials,
                                       &expiry);
    if (status != SEC_E_OK) {
        set_status_error(error, error_size, "AcquireCredentialsHandle", status);
        free(native);
        return 0;
    }
    native->credentials_valid = 1;

    memset(&output_buffer, 0, sizeof(output_buffer));
    output_buffer.BufferType = SECBUFFER_TOKEN;
    output_desc.ulVersion = SECBUFFER_VERSION;
    output_desc.cBuffers = 1U;
    output_desc.pBuffers = &output_buffer;

    status = InitializeSecurityContextA(&native->credentials,
                                        NULL,
                                        (SEC_CHAR *)host,
                                        request_flags,
                                        0U,
                                        SECURITY_NATIVE_DREP,
                                        NULL,
                                        0U,
                                        &native->context,
                                        &output_desc,
                                        &context_attributes,
                                        &expiry);

    if (status != SEC_I_CONTINUE_NEEDED) {
        set_status_error(error, error_size, "InitializeSecurityContext", status);
        FreeCredentialsHandle(&native->credentials);
        free(native);
        return 0;
    }
    native->context_valid = 1;

    if (output_buffer.pvBuffer != NULL && output_buffer.cbBuffer > 0U) {
        if (!send_all(socket_value,
                      (const unsigned char *)output_buffer.pvBuffer,
                      output_buffer.cbBuffer)) {
            FreeContextBuffer(output_buffer.pvBuffer);
            DeleteSecurityContext(&native->context);
            FreeCredentialsHandle(&native->credentials);
            free(native);
            set_error(error, error_size, "unable to send TLS handshake data");
            return 0;
        }
        FreeContextBuffer(output_buffer.pvBuffer);
    }

    for (;;) {
        int received;

        if (input_length == sizeof(input_data)) {
            set_error(error, error_size, "TLS handshake input exceeded buffer");
            DeleteSecurityContext(&native->context);
            FreeCredentialsHandle(&native->credentials);
            free(native);
            return 0;
        }

        received = recv(socket_value,
                        (char *)input_data + input_length,
                        (int)(sizeof(input_data) - input_length),
                        0);
        if (received <= 0) {
            set_error(error, error_size, "TLS peer closed during handshake");
            DeleteSecurityContext(&native->context);
            FreeCredentialsHandle(&native->credentials);
            free(native);
            return 0;
        }
        input_length += (size_t)received;

        memset(input_buffers, 0, sizeof(input_buffers));
        input_buffers[0].BufferType = SECBUFFER_TOKEN;
        input_buffers[0].pvBuffer = input_data;
        input_buffers[0].cbBuffer = (unsigned long)input_length;
        input_buffers[1].BufferType = SECBUFFER_EMPTY;

        input_desc.ulVersion = SECBUFFER_VERSION;
        input_desc.cBuffers = 2U;
        input_desc.pBuffers = input_buffers;

        memset(&output_buffer, 0, sizeof(output_buffer));
        output_buffer.BufferType = SECBUFFER_TOKEN;

        status = InitializeSecurityContextA(&native->credentials,
                                            &native->context,
                                            (SEC_CHAR *)host,
                                            request_flags,
                                            0U,
                                            SECURITY_NATIVE_DREP,
                                            &input_desc,
                                            0U,
                                            NULL,
                                            &output_desc,
                                            &context_attributes,
                                            &expiry);

        if (output_buffer.pvBuffer != NULL && output_buffer.cbBuffer > 0U) {
            if (!send_all(socket_value,
                          (const unsigned char *)output_buffer.pvBuffer,
                          output_buffer.cbBuffer)) {
                FreeContextBuffer(output_buffer.pvBuffer);
                DeleteSecurityContext(&native->context);
                FreeCredentialsHandle(&native->credentials);
                free(native);
                set_error(error, error_size, "unable to send TLS handshake data");
                return 0;
            }
            FreeContextBuffer(output_buffer.pvBuffer);
        }

        if (status == SEC_E_OK) {
            break;
        }

        if (status == SEC_E_INCOMPLETE_MESSAGE) {
            continue;
        }

        if (status == SEC_I_INCOMPLETE_CREDENTIALS) {
            /*
             * The peer requested a client certificate. Rictus does not use
             * certificate-based client authentication for IRC; continue the
             * Schannel handshake without supplying one.
             */
            status = SEC_I_CONTINUE_NEEDED;
        }

        if (status != SEC_I_CONTINUE_NEEDED) {
            set_status_error(error, error_size, "TLS handshake", status);
            DeleteSecurityContext(&native->context);
            FreeCredentialsHandle(&native->credentials);
            free(native);
            return 0;
        }

        if (input_buffers[1].BufferType == SECBUFFER_EXTRA &&
            input_buffers[1].cbBuffer > 0U) {
            size_t extra = input_buffers[1].cbBuffer;
            memmove(input_data, input_data + input_length - extra, extra);
            input_length = extra;
        } else {
            input_length = 0U;
        }
    }

    tls->native = native;
    return 1;
}

void rictus_tls_close(rictus_tls_connection *tls)
{
    rictus_schannel_connection *native;

    if (tls == NULL || tls->native == NULL) {
        return;
    }

    native = (rictus_schannel_connection *)tls->native;
    if (native->context_valid) {
        DeleteSecurityContext(&native->context);
    }
    if (native->credentials_valid) {
        FreeCredentialsHandle(&native->credentials);
    }

    free(native);
    tls->native = NULL;
}
