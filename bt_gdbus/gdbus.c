
#include <stdio.h>
#include <stdbool.h>
#include <stdint.h>

#include "gdbus.h"
#include "gdbus_common.h"

#define TAG						"[GDBUS]"

#define AGENT_PATH				"/custom/agent/displayyesno"
#define PROFILE_PATH			"/custom/profile/spp"
#define SPP_UUID				"00001101-0000-1000-8000-00805F9B34FB"

GDBusConnection *gdbus_con = NULL;
GMainLoop *gdbus_loop = NULL;
int gdbus_spp_fd = -1;

/* Introspection XML for Agent1 and Profile1 */
static const gchar gdbus_introspection_xml[] =
	"<node>"
	"    <interface name='org.bluez.Agent1'>"
	"        <method name='RequestConfirmation'>"
	"            <arg type='o' name='device' direction='in' />"
	"            <arg type='u' name='passkey' direction='in' />"
	"        </method>"
	"        <method name='AuthorizeService'>"
	"            <arg type='o' name='device' direction='in' />"
	"            <arg type='s' name='uuid' direction='in' />"
	"        </method>"
	"        <method name='Cancel' />"
	"    </interface>"
	""
	"    <interface name='org.bluez.Profile1'>"
	"        <method name='Release' />"
	"        <method name='NewConnection'>"
	"            <arg type='o' name='device' direction='in' />"
	"            <arg type='h' name='fd' direction='in' />"
	"            <arg type='a{sv}' name='properties' direction='in' />"
	"        </method>"
	"        <method name='RequestDisconnection'>"
	"            <arg type='o' name='device' direction='in' />"
	"        </method>"
	"    </interface>"
	"</node>";

/* Implement this function in your code to answer for pairing requests.
 * Default implementation accept all pairing requests.
 */
__attribute__((weak))
bool gdbus_pairing_response(const char *device_path, uint32_t passkey) {
	GDBUS_UNUSED(device_path);
	GDBUS_UNUSED(passkey);

	return true;
}

/* Implement this function in your code to handle SPP incoming data.
 */
__attribute__((weak))
void gdbus_spp_on_data_available(uint8_t *buf, uint32_t len) {
	GDBUS_UNUSED(buf);
	GDBUS_UNUSED(len);
}

/* SPP send function */
void gdbus_spp_write(const char *data, size_t len) {
	if (gdbus_spp_fd < 0) {
		LOGW(TAG, "Cannot write, no active connection\n");
		return;
	}

	ssize_t written = write(gdbus_spp_fd, data, len);
	if (written < 0) {
		LOGE(TAG, "Write error: %s\n", strerror(errno));
	}
}

/* Callback when data is received on the SPP socket */
gboolean gdbus_spp_read_callback(GIOChannel *source, GIOCondition condition, gpointer data) {
	GDBUS_UNUSED(data);

	if (condition & G_IO_HUP || condition & G_IO_ERR) {
		LOGW(TAG, "Connection closed or error on socket\n");
		gdbus_spp_fd = -1;
		return FALSE;
	}

	gchar buf[GDBUS_SPP_READ_BUF_SIZE];
	gsize bytes_read = 0;
	GError *error = NULL;

	GIOStatus status = g_io_channel_read_chars(source, buf, sizeof(buf), &bytes_read, &error);
	if (status == G_IO_STATUS_NORMAL && bytes_read > 0) {
		gdbus_spp_on_data_available((uint8_t *)buf, (uint32_t)bytes_read);
	} else {
		if (error) {
			LOGE(TAG, "Error: %s\n", error->message);
			g_error_free(error);
		}
	}

	return TRUE; // keep watch active
}

/* Handle D-Bus method calls */
void gdbus_handle_method_call(GDBusConnection *con,
		const gchar *sender,
		const gchar *object_path,
		const gchar *interface_name,
		const gchar *method_name,
		GVariant *parameters,
		GDBusMethodInvocation *invocation,
		gpointer user_data) {
	GDBUS_UNUSED(con);
	GDBUS_UNUSED(sender);
	GDBUS_UNUSED(object_path);
	GDBUS_UNUSED(user_data);

	/* Agent1 interface methods */
	if (g_strcmp0(interface_name, "org.bluez.Agent1") == 0) {
		if (g_strcmp0(method_name, "RequestConfirmation") == 0) {
			const gchar *device_path;
			guint32 passkey;
			g_variant_get(parameters, "(&ou)", &device_path, &passkey);

			LOGI(TAG, "RequestConfirmation triggered for device %s\n", device_path);
			LOGI(TAG, "\tPasskey: %06u\n", passkey);

			// get user response
			bool response = gdbus_pairing_response(device_path, passkey);
			if (response) {
				LOGI(TAG, "\tAccepted\n");
				g_dbus_method_invocation_return_value(invocation, NULL);
			} else {
				LOGI(TAG, "\tRejected\n");
				g_dbus_method_invocation_return_dbus_error(invocation,
						"org.bluez.Error.Rejected",
						"Pairing rejected by user");
			}
		}
		else if (g_strcmp0(method_name, "AuthorizeService") == 0) {
			LOGI(TAG, "AuthorizeService triggered\n");
			g_dbus_method_invocation_return_value(invocation, NULL);
		}
		else if (g_strcmp0(method_name, "Cancel") == 0) {
			LOGI(TAG, "Pairing operation canceled\n");
			g_dbus_method_invocation_return_value(invocation, NULL);
		}
	}
	/* Profile1 interface methods */
	else if (g_strcmp0(interface_name, "org.bluez.Profile1") == 0) {
		if (g_strcmp0(method_name, "NewConnection") == 0) {
			const gchar *device_path;
			GUnixFDList *fd_list;
			gint fd_index;

			// Extract object path, file descriptor handle (h), without properties
			g_variant_get(parameters, "(oha{sv})", &device_path, &fd_index, NULL);

			fd_list = g_dbus_message_get_unix_fd_list(g_dbus_method_invocation_get_message(invocation));
			if (fd_list != NULL) {
				GError *error = NULL;
				gdbus_spp_fd = g_unix_fd_list_get(fd_list, fd_index, &error);
				if (error) {
					LOGE(TAG, "Failed to extract file descriptor: %s\n", error->message);
					g_error_free(error);
					g_dbus_method_invocation_return_error(invocation,
							G_IO_ERROR,
							G_IO_ERROR_FAILED,
							"FD extraction failed");
					return;
				}
			}

			LOGI(TAG, "New connection established from device: %s (FD: %d)\n", device_path, gdbus_spp_fd);

			// Register GLib IO channel watch to listen for inbound data asynchronously
			GIOChannel *channel = g_io_channel_unix_new(gdbus_spp_fd);
			g_io_channel_set_encoding(channel, NULL, NULL);
			g_io_channel_set_buffered(channel, FALSE);
			g_io_add_watch(channel,
					G_IO_IN | G_IO_HUP | G_IO_ERR,
					(GIOFunc)gdbus_spp_read_callback,
					NULL);
			g_dbus_method_invocation_return_value(invocation, NULL);
		}
		else if (g_strcmp0(method_name, "RequestDisconnection") == 0) {
			const gchar *device_path;
			g_variant_get(parameters, "(&o)", &device_path);
			LOGI(TAG, "Device %s disconnected\n", device_path);
			if (gdbus_spp_fd >= 0) {
				close(gdbus_spp_fd);
				gdbus_spp_fd = -1;
			}
			g_dbus_method_invocation_return_value(invocation, NULL);
		}
		else if (g_strcmp0(method_name, "Release") == 0) {
			LOGI(TAG, "Release triggered\n");
			g_dbus_method_invocation_return_value(invocation, NULL);
		}
	}
}

const GDBusInterfaceVTable gdbus_interface_vtable = {
	.method_call = gdbus_handle_method_call,
	.get_property = NULL,
	.set_property = NULL,
};

int gdbus_call_bluez(const gchar *interface, const gchar *method, GVariant *params) {
	GError *error = NULL;

	GVariant *result = g_dbus_connection_call_sync(
		gdbus_con, "org.bluez", "/org/bluez",
		interface, method, params, NULL,
		G_DBUS_CALL_FLAGS_NONE, -1, NULL, &error
	);

	if (error) {
		LOGE(TAG, "Failed to call %s.%s method: %s\n", interface, method, error->message);
		g_error_free(error);
		return -1;
	}

	if (result) {
		g_variant_unref(result);
	}

	return 0;
}

int gdbus_set_adapter_powered(const char *adapter_path, bool powered) {
	GError *error = NULL;

	GVariant *result = g_dbus_connection_call_sync(
		gdbus_con, "org.bluez",
		adapter_path,
		"org.freedesktop.DBus.Properties",
		"Set",
		g_variant_new("(ssv)",
			"org.bluez.Adapter1",
			"Powered",
			g_variant_new_boolean(powered ? TRUE : FALSE)
		),
		NULL,
		G_DBUS_CALL_FLAGS_NONE, -1, NULL, &error
	);

	if (error) {
		LOGE(TAG, "Failed to set %s power %s: %s\n",
				adapter_path,
				powered ? "on" : "off",
				error->message);
		g_error_free(error);
		return -1;
	} else {
		LOGI(TAG, "Set %s power %s\n", adapter_path, powered ? "on" : "off");
	}

	if (result) {
		g_variant_unref(result);
	}

	return 0;
}

int gdbus_set_adapter_discoverable(const char *adapter_path, bool discoverable) {
	GError *error = NULL;

	GVariant *result = g_dbus_connection_call_sync(
		gdbus_con, "org.bluez",
		adapter_path,
		"org.freedesktop.DBus.Properties",
		"Set",
		g_variant_new("(ssv)",
			"org.bluez.Adapter1",
			"Discoverable",
			g_variant_new_boolean(discoverable ? TRUE : FALSE)
		),
		NULL,
		G_DBUS_CALL_FLAGS_NONE, -1, NULL, &error
	);

	if (error) {
		LOGE(TAG, "Failed to set %s discoverable %s: %s\n",
				adapter_path,
				discoverable ? "on" : "off",
				error->message);
		g_error_free(error);
		return -1;
	} else {
		LOGI(TAG, "Set %s discoverable %s\n", adapter_path, discoverable ? "on" : "off");
	}

	if (result) {
		g_variant_unref(result);
	}

	return 0;
}

int gdbus_init() {
	GError *error = NULL;

	gdbus_con = g_bus_get_sync(G_BUS_TYPE_SYSTEM, NULL, &error);
	if (error) {
		LOGE(TAG, "Error connecting to system bus: %s\n", error->message);
		g_error_free(error);
		return -1;
	}

	gdbus_loop = g_main_loop_new(NULL, FALSE);
	GDBusNodeInfo *node_info = g_dbus_node_info_new_for_xml(gdbus_introspection_xml, &error);

	// Register Object paths for both Agent and Profile
	g_dbus_connection_register_object(gdbus_con, AGENT_PATH, node_info->interfaces[0], &gdbus_interface_vtable, NULL, NULL, &error);
	g_dbus_connection_register_object(gdbus_con, PROFILE_PATH, node_info->interfaces[1], &gdbus_interface_vtable, NULL, NULL, &error);

	LOGI(TAG, "Registering Agent and SPP Profile ...\n");
	// Register DisplayYesNo Agent
	gdbus_call_bluez("org.bluez.AgentManager1",
			"RegisterAgent",
			g_variant_new("(os)", AGENT_PATH, "DisplayYesNo"));
	gdbus_call_bluez("org.bluez.AgentManager1",
			"RequestDefaultAgent",
			g_variant_new("(o)", AGENT_PATH));

	// Register SPP Profile UUID
	GVariantBuilder builder;
	g_variant_builder_init(&builder, G_VARIANT_TYPE("a{sv}"));
	g_variant_builder_add(&builder, "{sv}", "Name", g_variant_new_string("SerialPort"));
	gdbus_call_bluez("org.bluez.ProfileManager1",
			"RegisterProfile",
			g_variant_new("(os@a{sv})",
					PROFILE_PATH,
					SPP_UUID,
					g_variant_builder_end(&builder)));

	LOGI(TAG, "Bluetooth Service Active\n");

	return 0;
}

int gdbus_run() {
	gdbus_set_adapter_powered("/org/bluez/hci0", true);
	gdbus_set_adapter_discoverable("/org/bluez/hci0", true);

	LOGI(TAG, "Ready for Pairing and SPP data exchange\n");
	g_main_loop_run(gdbus_loop);

	return 0;
}
