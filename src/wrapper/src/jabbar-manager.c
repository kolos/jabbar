#include "jabbar-manager.h"
#include "Common.h"
#include "JabraDeviceConfig.h"
#include <stdio.h>

struct _JabBarManager {
    GObject parent_instance;
};

G_DEFINE_TYPE(JabBarManager, jabbar_manager, G_TYPE_OBJECT)

enum {
    SIGNAL_DEVICE_ADDED,
    SIGNAL_DEVICE_REMOVED,
    SIGNAL_HEAD_STATUS_CHANGED,
    SIGNAL_BATTERY_STATUS_CHANGED,
    N_SIGNALS
};

static guint signals[N_SIGNALS];
static JabBarManager *singleton = NULL;

typedef struct {
    unsigned short deviceID;
    char *deviceName;
} DeviceInfoData;

typedef struct {
    unsigned short deviceID;
    gboolean detected;
} HeadStatusData;

typedef struct {
    unsigned short deviceID;
    int level;
    gboolean charging;
} BatteryStatusData;

static gboolean emit_device_added(gpointer data) {
    DeviceInfoData *info = (DeviceInfoData *)data;
    if (singleton) {
        g_signal_emit(singleton, signals[SIGNAL_DEVICE_ADDED], 0, (guint)info->deviceID, info->deviceName);
    }
    g_free(info->deviceName);
    g_free(info);
    return G_SOURCE_REMOVE;
}

static gboolean emit_device_removed(gpointer data) {
    unsigned short deviceID = GPOINTER_TO_UINT(data);
    if (singleton) {
        g_signal_emit(singleton, signals[SIGNAL_DEVICE_REMOVED], 0, (guint)deviceID);
    }
    return G_SOURCE_REMOVE;
}

static gboolean emit_head_status(gpointer data) {
    HeadStatusData *status = (HeadStatusData *)data;
    if (singleton) {
        g_signal_emit(singleton, signals[SIGNAL_HEAD_STATUS_CHANGED], 0, (guint)status->deviceID, status->detected);
    }
    g_free(status);
    return G_SOURCE_REMOVE;
}

static gboolean emit_battery_status(gpointer data) {
    BatteryStatusData *status = (BatteryStatusData *)data;
    if (singleton) {
        g_signal_emit(singleton, signals[SIGNAL_BATTERY_STATUS_CHANGED], 0, (guint)status->deviceID, (gint)status->level, status->charging);
    }
    g_free(status);
    return G_SOURCE_REMOVE;
}

static void on_head_detection(unsigned short deviceID, const HeadDetectionStatus status) {
    HeadStatusData *data = g_new(HeadStatusData, 1);
    data->deviceID = deviceID;
    data->detected = status.leftOn || status.rightOn;
    g_idle_add(emit_head_status, data);
}

static void on_battery_status(unsigned short deviceID, Jabra_BatteryStatus* batteryStatus) {
    BatteryStatusData *data = g_new(BatteryStatusData, 1);
    data->deviceID = deviceID;
    data->level = batteryStatus->levelInPercent;
    data->charging = batteryStatus->charging;
    g_idle_add(emit_battery_status, data);
    Jabra_FreeBatteryStatus(batteryStatus);
}

static void on_device_attached(Jabra_DeviceInfo deviceInfo) {
    if (deviceInfo.isDongle) return;

    DeviceInfoData *data = g_new(DeviceInfoData, 1);
    data->deviceID = deviceInfo.deviceID;
    data->deviceName = g_strdup(deviceInfo.deviceName);
    g_idle_add(emit_device_added, data);

    
    Jabra_SetHeadDetectionStatusListener(deviceInfo.deviceID, on_head_detection);

    Jabra_BatteryStatus* batteryStatus = NULL;
    if (Jabra_GetBatteryStatusV2(deviceInfo.deviceID, &batteryStatus) == Return_Ok) {
        on_battery_status(deviceInfo.deviceID, batteryStatus);
    }
}

static void on_device_removed(unsigned short deviceID) {
    g_idle_add(emit_device_removed, GUINT_TO_POINTER((guint)deviceID));
}

static void jabbar_manager_dispose(GObject *object) {
    G_OBJECT_CLASS(jabbar_manager_parent_class)->dispose(object);
}

static void jabbar_manager_class_init(JabBarManagerClass *klass) {
    GObjectClass *object_class = G_OBJECT_CLASS(klass);
    object_class->dispose = jabbar_manager_dispose;

    signals[SIGNAL_DEVICE_ADDED] = g_signal_new("device-added",
        G_TYPE_FROM_CLASS(klass),
        G_SIGNAL_RUN_LAST,
        0, NULL, NULL, NULL,
        G_TYPE_NONE,
        2, G_TYPE_UINT, G_TYPE_STRING);

    signals[SIGNAL_DEVICE_REMOVED] = g_signal_new("device-removed",
        G_TYPE_FROM_CLASS(klass),
        G_SIGNAL_RUN_LAST,
        0, NULL, NULL, NULL,
        G_TYPE_NONE,
        1, G_TYPE_UINT);

    signals[SIGNAL_HEAD_STATUS_CHANGED] = g_signal_new("head-status-changed",
        G_TYPE_FROM_CLASS(klass),
        G_SIGNAL_RUN_LAST,
        0, NULL, NULL, NULL,
        G_TYPE_NONE,
        2, G_TYPE_UINT, G_TYPE_BOOLEAN);

    signals[SIGNAL_BATTERY_STATUS_CHANGED] = g_signal_new("battery-status-changed",
        G_TYPE_FROM_CLASS(klass),
        G_SIGNAL_RUN_LAST,
        0, NULL, NULL, NULL,
        G_TYPE_NONE,
        3, G_TYPE_UINT, G_TYPE_INT, G_TYPE_BOOLEAN);
}

static void jabbar_manager_init(JabBarManager *self) {
    singleton = self;
}

JabBarManager *jabbar_manager_new(void) {
    return g_object_new(JABBAR_TYPE_MANAGER, NULL);
}

void jabbar_manager_start(JabBarManager *self) {
    Jabra_SetAppID("cinnamon-applet");
    Jabra_InitializeV2(NULL, on_device_attached, on_device_removed, NULL, NULL, false, NULL);
    Jabra_RegisterBatteryStatusUpdateCallbackV2(on_battery_status);
}

void jabbar_manager_stop(JabBarManager *self) {
    Jabra_Uninitialize();
}
