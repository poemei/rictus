#include <stdio.h>
#include <string.h>

#include "rictus_irc.h"
#include "rictus_command.h"
#include "rictus_dispatch.h"
#include "rictus_event.h"
#include "rictus_irc_message.h"

#define IRC_LINE_MAX 1024
#define IRC_READ_MAX 16384
#define IRC_AUTH_RAW_MAX (RICTUS_CONFIG_USERNAME_MAX * 2U + RICTUS_CONFIG_PASSWORD_MAX + 2U)
#define IRC_AUTH_B64_MAX (((IRC_AUTH_RAW_MAX + 2U) / 3U) * 4U + 1U)
#define RICTUS_HOST_COMMAND_MAX 32U

typedef struct rictus_host_command_entry {
    char name[RICTUS_MODULE_COMMAND_NAME_MAX];
    rictus_module_command_handler_fn handler;
    void *handler_context;
} rictus_host_command_entry;

static rictus_host_command_entry g_host_commands[RICTUS_HOST_COMMAND_MAX];
static size_t g_host_command_count = 0U;
static rictus_tls_connection *g_host_tls = NULL;

static int send_line(rictus_tls_connection *tls,
                     const char *line,
                     char *error,
                     size_t error_size);
static char g_host_channel[RICTUS_IRC_PARAM_MAX];
static rictus_irc_online_fn g_online_callback = NULL;
static void *g_online_context = NULL;

static int host_send_message(const char *message)
{
    char wire[IRC_LINE_MAX];
    char error[256];

    if (message == NULL || message[0] == '\0' ||
        g_host_tls == NULL || g_host_channel[0] == '\0') {
        return 0;
    }

    if (snprintf(wire, sizeof(wire), "PRIVMSG %s :%s",
                 g_host_channel, message) < 0) {
        return 0;
    }

    return send_line(g_host_tls, wire, error, sizeof(error));
}

static int host_register_command(const char *name,
                                 rictus_module_command_handler_fn handler,
                                 void *handler_context)
{
    size_t index;

    if (name == NULL || name[0] == '\0' || handler == NULL ||
        strlen(name) >= RICTUS_MODULE_COMMAND_NAME_MAX) {
        return 0;
    }

    for (index = 0U; index < g_host_command_count; ++index) {
        if (strcmp(g_host_commands[index].name, name) == 0) {
            return 0;
        }
    }

    if (g_host_command_count >= RICTUS_HOST_COMMAND_MAX) {
        return 0;
    }

    (void)snprintf(g_host_commands[g_host_command_count].name,
                   sizeof(g_host_commands[g_host_command_count].name),
                   "%s", name);
    g_host_commands[g_host_command_count].handler = handler;
    g_host_commands[g_host_command_count].handler_context = handler_context;
    ++g_host_command_count;
    return 1;
}

static int host_unregister_command(const char *name, void *handler_context)
{
    size_t index;

    if (name == NULL) {
        return 0;
    }

    for (index = 0U; index < g_host_command_count; ++index) {
        if (strcmp(g_host_commands[index].name, name) == 0 &&
            g_host_commands[index].handler_context == handler_context) {
            g_host_commands[index] =
                g_host_commands[g_host_command_count - 1U];
            --g_host_command_count;
            return 1;
        }
    }

    return 0;
}

void rictus_irc_set_online_callback(rictus_irc_online_fn callback, void *context)
{
    g_online_callback = callback;
    g_online_context = context;
}

void rictus_irc_host_init(rictus_module_host_t *host)
{
    if (host != NULL) {
        host->send_message = host_send_message;
        host->register_command = host_register_command;
        host->unregister_command = host_unregister_command;
    }
}

static void set_error(char *error, size_t error_size, const char *message)
{
    if (error != NULL && error_size > 0U) {
        (void)snprintf(error, error_size, "%s", message);
    }
}

static int send_line(rictus_tls_connection *tls,
                     const char *line,
                     char *error,
                     size_t error_size)
{
    char wire[IRC_LINE_MAX + 3U];
    int length = snprintf(wire, sizeof(wire), "%s\r\n", line);

    if (length < 0 || (size_t)length >= sizeof(wire)) {
        set_error(error, error_size, "IRC command exceeds line limit");
        return 0;
    }

    return rictus_tls_send(tls, wire, (size_t)length, error, error_size);
}

static size_t base64_encode(const unsigned char *input,
                            size_t input_length,
                            char *output,
                            size_t output_size)
{
    static const char alphabet[] =
        "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
    size_t in = 0U;
    size_t out = 0U;

    while (in < input_length) {
        unsigned int value;
        size_t remaining = input_length - in;

        if (out + 4U >= output_size) {
            return 0U;
        }

        value = (unsigned int)input[in] << 16;
        if (remaining > 1U) {
            value |= (unsigned int)input[in + 1U] << 8;
        }
        if (remaining > 2U) {
            value |= (unsigned int)input[in + 2U];
        }

        output[out++] = alphabet[(value >> 18) & 0x3fU];
        output[out++] = alphabet[(value >> 12) & 0x3fU];
        output[out++] = remaining > 1U ? alphabet[(value >> 6) & 0x3fU] : '=';
        output[out++] = remaining > 2U ? alphabet[value & 0x3fU] : '=';
        in += remaining >= 3U ? 3U : remaining;
    }

    output[out] = '\0';
    return out;
}

static int send_sasl_plain(rictus_tls_connection *tls,
                           const rictus_irc_config *config,
                           char *error,
                           size_t error_size)
{
    unsigned char raw[IRC_AUTH_RAW_MAX];
    char encoded[IRC_AUTH_B64_MAX];
    size_t username_length = strlen(config->username);
    size_t password_length = strlen(config->password);
    size_t raw_length;
    size_t encoded_length;
    size_t offset = 0U;

    raw_length = username_length + 1U + username_length + 1U + password_length;
    if (raw_length > sizeof(raw)) {
        set_error(error, error_size, "SASL credentials exceed bounded buffer");
        return 0;
    }

    memcpy(raw, config->username, username_length);
    raw[username_length] = '\0';
    memcpy(raw + username_length + 1U, config->username, username_length);
    raw[username_length + 1U + username_length] = '\0';
    memcpy(raw + username_length + 1U + username_length + 1U,
           config->password,
           password_length);

    encoded_length = base64_encode(raw, raw_length, encoded, sizeof(encoded));
    if (encoded_length == 0U) {
        set_error(error, error_size, "unable to encode SASL credentials");
        return 0;
    }

    while (offset < encoded_length) {
        char command[IRC_LINE_MAX];
        size_t chunk = encoded_length - offset;

        if (chunk > 400U) {
            chunk = 400U;
        }

        (void)snprintf(command,
                       sizeof(command),
                       "AUTHENTICATE %.*s",
                       (int)chunk,
                       encoded + offset);
        if (!send_line(tls, command, error, error_size)) {
            return 0;
        }
        offset += chunk;
    }

    if ((encoded_length % 400U) == 0U) {
        return send_line(tls, "AUTHENTICATE +", error, error_size);
    }

    return 1;
}

static int contains_numeric(const char *line, const char *numeric)
{
    char needle[8];
    (void)snprintf(needle, sizeof(needle), " %s ", numeric);
    return strstr(line, needle) != NULL;
}

int rictus_irc_run(rictus_tls_connection *tls,
                   const rictus_irc_config *config,
                   char *error,
                   size_t error_size)
{
    unsigned char incoming[IRC_READ_MAX];
    char pending[IRC_READ_MAX + IRC_LINE_MAX];
    size_t pending_length = 0U;
    int sasl_requested = 0;
    int sasl_started = 0;
    int sasl_payload_sent = 0;
    int sasl_complete = 0;
    int join_sent = 0;
    int joined = 0;

    if (tls == NULL || config == NULL) {
        set_error(error, error_size, "invalid IRC session request");
        return 0;
    }

    g_host_tls = tls;
    (void)snprintf(g_host_channel, sizeof(g_host_channel), "%s", config->channel);

    if (!send_line(tls, "CAP LS 302", error, error_size)) {
        return 0;
    }

    {
        char command[IRC_LINE_MAX];
        (void)snprintf(command, sizeof(command), "NICK %s", config->username);
        if (!send_line(tls, command, error, error_size)) {
            return 0;
        }
        (void)snprintf(command,
                       sizeof(command),
                       "USER %s 0 * :Rictus",
                       config->username);
        if (!send_line(tls, command, error, error_size)) {
            return 0;
        }
    }

    puts("[INFO] IRC registration started.");

    for (;;) {
        size_t received = 0U;
        size_t consumed = 0U;

        if (!rictus_tls_receive(tls,
                                incoming,
                                sizeof(incoming),
                                &received,
                                error,
                                error_size)) {
            return 0;
        }

        if (pending_length + received > sizeof(pending)) {
            set_error(error, error_size, "IRC receive buffer exceeded");
            return 0;
        }

        memcpy(pending + pending_length, incoming, received);
        pending_length += received;

        while (consumed < pending_length) {
            char *newline = memchr(pending + consumed, '\n', pending_length - consumed);
            size_t line_length;
            char line[IRC_LINE_MAX];

            if (newline == NULL) {
                break;
            }

            line_length = (size_t)(newline - (pending + consumed));
            if (line_length > 0U && pending[consumed + line_length - 1U] == '\r') {
                --line_length;
            }
            if (line_length >= sizeof(line)) {
                set_error(error, error_size, "IRC line exceeds bounded buffer");
                return 0;
            }

            memcpy(line, pending + consumed, line_length);
            line[line_length] = '\0';
            consumed = (size_t)(newline - pending) + 1U;

            {
                rictus_irc_message message;
                rictus_event event;

                if (rictus_irc_message_parse(line, &message) &&
                    rictus_event_from_irc(&message, config->channel, &event)) {
                    rictus_dispatch_result dispatch;

                    rictus_event_observe(&event);
                    if (rictus_dispatch_event(&event, &dispatch)) {
                        rictus_dispatch_observe(&dispatch);

                        if (dispatch.action == RICTUS_DISPATCH_COMMAND) {
                            rictus_command command;
                            char response[IRC_LINE_MAX];
                            char wire[IRC_LINE_MAX];

                            if (rictus_command_parse(&dispatch, &command) &&
                                rictus_command_response(&command,
                                                        response,
                                                        sizeof(response))) {
                                const char *reply_target = event.type == RICTUS_EVENT_PRIVATE_MESSAGE
                                    ? config->username
                                    : event.target;

                                if (event.type == RICTUS_EVENT_PRIVATE_MESSAGE) {
                                    const char *bang = strchr(event.source, '!');
                                    size_t nick_length = bang == NULL
                                        ? strlen(event.source)
                                        : (size_t)(bang - event.source);

                                    if (nick_length == 0U || nick_length >= RICTUS_IRC_PARAM_MAX) {
                                        set_error(error,
                                                  error_size,
                                                  "invalid IRC private-message source");
                                        return 0;
                                    }

                                    {
                                        char nick[RICTUS_IRC_PARAM_MAX];
                                        memcpy(nick, event.source, nick_length);
                                        nick[nick_length] = '\0';
                                        reply_target = nick;

                                        (void)snprintf(wire,
                                                       sizeof(wire),
                                                       "PRIVMSG %s :%s",
                                                       reply_target,
                                                       response);
                                        if (!send_line(tls, wire, error, error_size)) {
                                            return 0;
                                        }
                                    }
                                } else {
                                    (void)snprintf(wire,
                                                   sizeof(wire),
                                                   "PRIVMSG %s :%s",
                                                   reply_target,
                                                   response);
                                    if (!send_line(tls, wire, error, error_size)) {
                                        return 0;
                                    }
                                }
                            }
                        }
                    }
                }
            }

            if (strncmp(line, "PING ", 5U) == 0) {
                char pong[IRC_LINE_MAX];
                (void)snprintf(pong, sizeof(pong), "PONG %s", line + 5U);
                if (!send_line(tls, pong, error, error_size)) {
                    return 0;
                }
                continue;
            }

            if (!sasl_requested && strstr(line, " CAP ") != NULL &&
                strstr(line, " LS ") != NULL && strstr(line, "sasl") != NULL) {
                if (!send_line(tls, "CAP REQ :sasl", error, error_size)) {
                    return 0;
                }
                sasl_requested = 1;
                continue;
            }

            if (sasl_requested && !sasl_started && strstr(line, " CAP ") != NULL &&
                strstr(line, " ACK ") != NULL && strstr(line, "sasl") != NULL) {
                if (!send_line(tls, "AUTHENTICATE PLAIN", error, error_size)) {
                    return 0;
                }
                sasl_started = 1;
                continue;
            }

            if (sasl_started && !sasl_payload_sent &&
                strcmp(line, "AUTHENTICATE +") == 0) {
                if (!send_sasl_plain(tls, config, error, error_size)) {
                    return 0;
                }
                sasl_payload_sent = 1;
                continue;
            }

            if (contains_numeric(line, "903")) {
                sasl_complete = 1;
                puts("[INFO] IRC SASL authentication successful.");
                if (!send_line(tls, "CAP END", error, error_size)) {
                    return 0;
                }
                continue;
            }

            if (contains_numeric(line, "904") ||
                contains_numeric(line, "905") ||
                contains_numeric(line, "906") ||
                contains_numeric(line, "907")) {
                set_error(error, error_size, "IRC SASL authentication failed");
                return 0;
            }

            if (sasl_complete && !join_sent && contains_numeric(line, "001")) {
                char command[IRC_LINE_MAX];
                (void)snprintf(command, sizeof(command), "JOIN %s", config->channel);
                if (!send_line(tls, command, error, error_size)) {
                    return 0;
                }
                join_sent = 1;
                continue;
            }

            if (join_sent && !joined && strstr(line, " JOIN ") != NULL &&
                strstr(line, config->channel) != NULL) {
                joined = 1;
                printf("[INFO] IRC joined %s.\n", config->channel);
                puts("[INFO] Rictus online.");
                if (g_online_callback != NULL) {
                    g_online_callback(g_online_context);
                }
                continue;
            }

            if (!sasl_requested && strstr(line, " CAP ") != NULL &&
                strstr(line, " LS ") != NULL && strstr(line, "sasl") == NULL &&
                strstr(line, " * ") == NULL) {
                set_error(error, error_size, "IRC server does not advertise SASL");
                return 0;
            }
        }

        if (consumed > 0U) {
            memmove(pending, pending + consumed, pending_length - consumed);
            pending_length -= consumed;
        }
    }
}
