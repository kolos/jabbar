const Applet = imports.ui.applet;
const Main = imports.ui.main;
const Settings = imports.ui.settings;
const GLib = imports.gi.GLib;
const GObject = imports.gi.GObject;
const GIRepository = imports.gi.GIRepository;
const Gio = imports.gi.Gio;

const uuid = "jabbar@kolos";
const appletPath = imports.ui.appletManager.appletMeta[uuid].path;

// Add current directory to typelib search path
GIRepository.Repository.prepend_search_path(appletPath);
GIRepository.Repository.prepend_library_path(appletPath);

let JabBar;
try {
    JabBar = imports.gi.JabBar;
} catch (e) {
    global.logError("Could not import JabBar: " + e);
}

class JabraApplet extends Applet.IconApplet {
    constructor(metadata, orientation, panel_height, instance_id) {
        super(orientation, panel_height, instance_id);

        this.set_applet_tooltip("JabBar");

        this.settings = new Settings.AppletSettings(this, uuid, instance_id);
        this.settings.bind("enable-notifications", "enableNotifications");
        this.settings.bind("enable-media-control", "enableMediaControl");
        this.settings.bind("critical-battery-level", "criticalBatteryLevel");
        this.settings.bind("icon-default", "iconDefault");
        this.settings.bind("icon-head-detected", "iconHeadDetected");
        this.settings.bind("icon-head-missing", "iconHeadMissing");

        // Set initial icon from settings
        this.set_applet_icon_name(this.iconDefault);

        if (JabBar) {
            try {
            this.manager = new JabBar.Manager();
                
                this.manager.connect('device-added', (manager, id, name) => {
                    this.deviceName = name;
                    this.updateTooltip();
                    if (this.enableNotifications) {
                        Main.notify("JabBar", `Device attached: ${name}`);
                    }
                });

                this.manager.connect('device-removed', (manager, id) => {
                    this.deviceName = null;
                    this.batteryLevel = null;
                    this.updateTooltip();
                    if (this.enableNotifications) {
                        Main.notify("JabBar", "Device removed");
                    }
                });

                this.manager.connect('battery-status-changed', (manager, id, level, charging) => {
                    this.batteryLevel = level;
                    this.charging = charging;
                    this.updateTooltip();
                    if (level <= this.criticalBatteryLevel && !charging) {
                        if (this.enableNotifications) {
                            Main.notify("JabBar", `Critical battery level: ${level}%`);
                        }
                    }
                });

                this.manager.connect('head-status-changed', (manager, id, detected) => {
                    if (detected) {
                        this.set_applet_icon_name(this.iconHeadDetected);
                        if (this.enableMediaControl) {
                            this.controlMedia("Play");
                        }
                    } else {
                        this.set_applet_icon_name(this.iconHeadMissing);
                        if (this.enableMediaControl) {
                            this.controlMedia("Pause");
                        }
                    }
                });

                this.manager.start();
            } catch (e) {
                global.logError("Error initializing JabBar Manager: " + e);
                this.set_applet_tooltip("JabBar Error: " + e.toString());
            }
        } else {
            this.set_applet_tooltip("JabBar: Library not found");
        }
    }

    updateTooltip() {
        if (this.deviceName) {
            let tooltip = `${this.deviceName}`;
            if (this.batteryLevel !== null && this.batteryLevel !== undefined) {
                tooltip += `\nBattery: ${this.batteryLevel}%`;
                if (this.charging) {
                    tooltip += " (Charging)";
                }
            }
            this.set_applet_tooltip(tooltip);
        } else {
            this.set_applet_tooltip("JabBar: No Device");
        }
    }

    controlMedia(command) {
        try {
            let bus = Gio.bus_get_sync(Gio.BusType.SESSION, null);
            let result = bus.call_sync(
                "org.freedesktop.DBus",
                "/org/freedesktop/DBus",
                "org.freedesktop.DBus",
                "ListNames",
                null, null, Gio.DBusCallFlags.NONE, -1, null
            );
            let names = result.deep_unpack()[0];
            for (let name of names) {
                if (name.startsWith("org.mpris.MediaPlayer2.")) {
                    bus.call(
                        name,
                        "/org/mpris/MediaPlayer2",
                        "org.mpris.MediaPlayer2.Player",
                        command,
                        null, null, Gio.DBusCallFlags.NONE, -1, null,
                        (conn, res) => {
                            try {
                                conn.call_finish(res);
                            } catch (e) {
                                global.logError("Error calling " + command + " on " + name + ": " + e);
                            }
                        }
                    );
                }
            }
        } catch (e) {
            global.logError("Error controlling media: " + e);
        }
    }

    on_applet_removed_from_panel() {
        if (this.manager) {
            this.manager.stop();
        }
    }
}

function main(metadata, orientation, panel_height, instance_id) {
    return new JabraApplet(metadata, orientation, panel_height, instance_id);
}
