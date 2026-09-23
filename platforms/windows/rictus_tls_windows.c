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
    SOCKET socket_value;
    SecPkgContext_StreamSizes sizes;
    unsigned char encrypted[65536];
    size_t encrypted_length;
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
    native->socket_value = socket_value;

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
                                            &native->context,
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
            if (input_buffers[1].BufferType == SECBUFFER_EXTRA &&
                input_buffers[1].cbBuffer > 0U) {
                size_t extra = input_buffers[1].cbBuffer;
                memcpy(native->encrypted, input_data + input_length - extra, extra);
                native->encrypted_length = extra;
            }
            break;
        }

        if (status == SEC_E_INCOMPLETE_MESSAGE) {
            continue;
        }

        if (status == SEC_I_INCOMPLETE_CREDENTIALS) {
            /*
             * Schannel has no client certificate to supply. Re-enter the
             * existing context immediately with the same peer token so
             * Schannel can continue anonymously; do not wait for another
             * network record that the peer has no reason to send.
             */
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
                                                &native->context,
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
        }

        if (status == SEC_E_OK) {
            if (input_buffers[1].BufferType == SECBUFFER_EXTRA &&
                input_buffers[1].cbBuffer > 0U) {
                size_t extra = input_buffers[1].cbBuffer;
                memcpy(native->encrypted, input_data + input_length - extra, extra);
                native->encrypted_length = extra;
            }
            break;
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

    status = QueryContextAttributesA(&native->context,
                                     SECPKG_ATTR_STREAM_SIZES,
                                     &native->sizes);
    if (status != SEC_E_OK) {
        set_status_error(error, error_size, "QueryContextAttributes", status);
        DeleteSecurityContext(&native->context);
        FreeCredentialsHandle(&native->credentials);
        free(native);
        return 0;
    }

    tls->native = native;
    return 1;
}

int rictus_tls_send(rictus_tls_connection *tls,
                    const void *data,
                    size_t length,
                    char *error,
                    size_t error_size)
{
    rictus_schannel_connection *native;
    const unsigned char *cursor = (const unsigned char *)data;

    if (tls == NULL || tls->native == NULL || (data == NULL && length > 0U)) {
        set_error(error, error_size, "invalid TLS send request");
        return 0;
    }

    native = (rictus_schannel_connection *)tls->native;
    while (length > 0U) {
        size_t chunk = length;
        size_t packet_size;
        unsigned char *packet;
        SecBuffer buffers[4];
        SecBufferDesc desc;
        SECURITY_STATUS status;

        if (chunk > native->sizes.cbMaximumMessage) {
            chunk = native->sizes.cbMaximumMessage;
        }

        packet_size = native->sizes.cbHeader + chunk + native->sizes.cbTrailer;
        packet = (unsigned char *)malloc(packet_size);
        if (packet == NULL) {
            set_error(error, error_size, "unable to allocate TLS output buffer");
            return 0;
        }

        memcpy(packet + native->sizes.cbHeader, cursor, chunk);
        memset(buffers, 0, sizeof(buffers));
        buffers[0].BufferType = SECBUFFER_STREAM_HEADER;
        buffers[0].pvBuffer = packet;
        buffers[0].cbBuffer = native->sizes.cbHeader;
        buffers[1].BufferType = SECBUFFER_DATA;
        buffers[1].pvBuffer = packet + native->sizes.cbHeader;
        buffers[1].cbBuffer = (unsigned long)chunk;
        buffers[2].BufferType = SECBUFFER_STREAM_TRAILER;
        buffers[2].pvBuffer = packet + native->sizes.cbHeader + chunk;
        buffers[2].cbBuffer = native->sizes.cbTrailer;
        buffers[3].BufferType = SECBUFFER_EMPTY;

        desc.ulVersion = SECBUFFER_VERSION;
        desc.cBuffers = 4U;
        desc.pBuffers = buffers;

        status = EncryptMessage(&native->context, 0U, &desc, 0U);
        if (status != SEC_E_OK) {
            free(packet);
            set_status_error(error, error_size, "EncryptMessage", status);
            return 0;
        }

        packet_size = buffers[0].cbBuffer + buffers[1].cbBuffer + buffers[2].cbBuffer;
        if (!send_all(native->socket_value, packet, packet_size)) {
            free(packet);
            set_error(error, error_size, "unable to send TLS data");
            return 0;
        }

        free(packet);
        cursor += chunk;
        length -= chunk;
    }

    return 1;
}

int rictus_tls_receive(rictus_tls_connection *tls,
                       void *buffer,
                       size_t buffer_size,
                       size_t *received,
                       char *error,
                       size_t error_size)
{
    rictus_schannel_connection *native;

    if (tls == NULL || tls->native == NULL || buffer == NULL ||
        buffer_size == 0U || received == NULL) {
        set_error(error, error_size, "invalid TLS receive request");
        return 0;
    }

    native = (rictus_schannel_connection *)tls->native;
    *received = 0U;

    for (;;) {
        SecBuffer buffers[4];
        SecBufferDesc desc;
        SECURITY_STATUS status;
        unsigned int i;

        if (native->encrypted_length == 0U) {
            int count = recv(native->socket_value,
                             (char *)native->encrypted,
                             (int)sizeof(native->encrypted),
                             0);
            if (count <= 0) {
                set_error(error, error_size, "TLS peer closed connection");
                return 0;
            }
            native->encrypted_length = (size_t)count;
        }

        memset(buffers, 0, sizeof(buffers));
        buffers[0].BufferType = SECBUFFER_DATA;
        buffers[0].pvBuffer = native->encrypted;
        buffers[0].cbBuffer = (unsigned long)native->encrypted_length;
        buffers[1].BufferType = SECBUFFER_EMPTY;
        buffers[2].BufferType = SECBUFFER_EMPTY;
        buffers[3].BufferType = SECBUFFER_EMPTY;

        desc.ulVersion = SECBUFFER_VERSION;
        desc.cBuffers = 4U;
        desc.pBuffers = buffers;

        status = DecryptMessage(&native->context, &desc, 0U, NULL);
        if (status == SEC_E_INCOMPLETE_MESSAGE) {
            int count;
            if (native->encrypted_length == sizeof(native->encrypted)) {
                set_error(error, error_size, "TLS input exceeded buffer");
                return 0;
            }
            count = recv(native->socket_value,
                         (char *)native->encrypted + native->encrypted_length,
                         (int)(sizeof(native->encrypted) - native->encrypted_length),
                         0);
            if (count <= 0) {
                set_error(error, error_size, "TLS peer closed connection");
                return 0;
            }
            native->encrypted_length += (size_t)count;
            continue;
        }
        if (status != SEC_E_OK) {
            set_status_error(error, error_size, "DecryptMessage", status);
            return 0;
        }

        for (i = 0U; i < 4U; ++i) {
            if (buffers[i].BufferType == SECBUFFER_DATA && buffers[i].cbBuffer > 0U) {
                size_t count = buffers[i].cbBuffer;
                if (count > buffer_size) {
                    count = buffer_size;
                }
                memcpy(buffer, buffers[i].pvBuffer, count);
                *received = count;
            }
        }

        for (i = 0U; i < 4U; ++i) {
            if (buffers[i].BufferType == SECBUFFER_EXTRA && buffers[i].cbBuffer > 0U) {
                size_t extra = buffers[i].cbBuffer;
                memmove(native->encrypted,
                        native->encrypted + native->encrypted_length - extra,
                        extra);
                native->encrypted_length = extra;
                break;
            }
        }
        if (i == 4U) {
            native->encrypted_length = 0U;
        }

        if (*received > 0U) {
            return 1;
        }
    }
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
