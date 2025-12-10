#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dbus/dbus.h>

/**
 * Helper function to send a method call to all available MPRIS media players.
 * 
 * @param method_name The name of the method to call (e.g., "Play", "Pause")
 */
static void send_mpris_command(const char *method_name) {
    DBusError err;
    DBusConnection *conn;
    DBusMessage *msg;
    DBusMessage *reply;
    DBusMessageIter args;
    DBusMessageIter sub_iter;
    char *name;
    int current_type;

    dbus_error_init(&err);

    // Connect to the session bus
    conn = dbus_bus_get(DBUS_BUS_SESSION, &err);
    if (dbus_error_is_set(&err)) {
        fprintf(stderr, "DBus Connection Error (%s)\n", err.message);
        dbus_error_free(&err);
        return;
    }
    if (NULL == conn) {
        return;
    }

    // Create a method call to list names on the bus
    msg = dbus_message_new_method_call("org.freedesktop.DBus",      // Target service
                                       "/org/freedesktop/DBus",     // Object path
                                       "org.freedesktop.DBus",      // Interface
                                       "ListNames");                // Method
    
    if (NULL == msg) {
        fprintf(stderr, "Message Null\n");
        return;
    }

    // Send the message and block waiting for a reply
    reply = dbus_connection_send_with_reply_and_block(conn, msg, -1, &err);
    dbus_message_unref(msg);

    if (dbus_error_is_set(&err)) {
        fprintf(stderr, "DBus ListNames Error (%s)\n", err.message);
        dbus_error_free(&err);
        return;
    }

    if (NULL == reply) {
        return;
    }

    // Read the arguments from the reply
    if (!dbus_message_iter_init(reply, &args)) {
        fprintf(stderr, "Message has no arguments!\n");
        dbus_message_unref(reply);
        return;
    }

    // The result should be an array of strings
    if (dbus_message_iter_get_arg_type(&args) != DBUS_TYPE_ARRAY) {
        fprintf(stderr, "Argument is not an array!\n");
        dbus_message_unref(reply);
        return;
    }

    dbus_message_iter_recurse(&args, &sub_iter);

    // Iterate over the array elements
    while ((current_type = dbus_message_iter_get_arg_type(&sub_iter)) != DBUS_TYPE_INVALID) {
        if (current_type == DBUS_TYPE_STRING) {
            dbus_message_iter_get_basic(&sub_iter, &name);
            
            // Check if the service name starts with "org.mpris.MediaPlayer2."
            if (strncmp(name, "org.mpris.MediaPlayer2.", 23) == 0) {
                // Found a media player, send the command
                DBusMessage *cmd_msg = dbus_message_new_method_call(name,
                                                   "/org/mpris/MediaPlayer2",
                                                   "org.mpris.MediaPlayer2.Player",
                                                   method_name);
                
                if (cmd_msg) {
                    // Send the command asynchronously
                    dbus_connection_send(conn, cmd_msg, NULL);
                    dbus_message_unref(cmd_msg);
                }
            }
        }
        dbus_message_iter_next(&sub_iter);
    }

    dbus_message_unref(reply);
    dbus_connection_flush(conn);
}

void play_media() {
    printf("Resuming media playback via DBus...\n");
    send_mpris_command("Play");
}

void pause_media() {
    printf("Pausing media playback via DBus...\n");
    send_mpris_command("Pause");
}
