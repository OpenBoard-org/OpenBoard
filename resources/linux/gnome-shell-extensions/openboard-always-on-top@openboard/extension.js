import St from 'gi://St';
import Gio from 'gi://Gio';
import GLib from 'gi://GLib';
import * as Main from 'resource:///org/gnome/shell/ui/main.js';
import * as PanelMenu from 'resource:///org/gnome/shell/ui/panelMenu.js';
import { Extension } from 'resource:///org/gnome/shell/extensions/extension.js';

export default class OpenBoardAlwaysOnTop extends Extension {
    constructor(metadata) {
        super(metadata);
        this._indicator = null;
        this._exported = null;
        this._ownerId = 0;
        this._connection = null;
        try {
            log('OpenBoardAlwaysOnTop: constructor metadata=' + JSON.stringify(metadata));
        } catch (e) { log('OpenBoardAlwaysOnTop: constructor JSON error: ' + e); }
    }

    enable() {
        try {
            log('OpenBoardAlwaysOnTop: enable (class)');
            this._indicator = new PanelMenu.Button(0.0, this.metadata.name || 'OpenBoard', false);

            log('OpenBoardAlwaysOnTop: created PanelMenu.Button');

            const icon = new St.Icon({
                icon_name: 'face-laugh-symbolic',
                style_class: 'system-status-icon',
            });
            this._indicator.add_child(icon);

            log('OpenBoardAlwaysOnTop: added icon to indicator');

            Main.panel.addToStatusArea(this.metadata.uuid || 'openboard-always-on-top', this._indicator);
            log('OpenBoardAlwaysOnTop: panel indicator added');

            log('OpenBoardAlwaysOnTop: starting DBus setup');

            // Prepare a simple DBus interface that exposes SetAlwaysOnTop(match:string, enable:boolean) -> result:boolean
            try {
                const INTROSPECTION_XML = `
<node>
  <interface name="org.gnome.Shell.Extensions.OpenBoard">
    <method name="SetAlwaysOnTop">
      <arg type="s" name="match" direction="in"/>
      <arg type="b" name="enable" direction="in"/>
      <arg type="b" name="result" direction="out"/>
    </method>
  </interface>
</node>`;

                let nodeInfo = Gio.DBusNodeInfo.new_for_xml(INTROSPECTION_XML);
                log('OpenBoardAlwaysOnTop: parsed introspection XML');
                try { log('OpenBoardAlwaysOnTop: nodeInfo.interfaces.length=' + nodeInfo.interfaces.length); } catch (e) {}
                let ifaceInfo = nodeInfo.interfaces[0];
                try { log('OpenBoardAlwaysOnTop: iface name=' + ifaceInfo.name); } catch (e) {}

                let self = this;

                let serviceObj = {
                    // Accept both forms: (invocation, match, enable) or (match, enable)
                    SetAlwaysOnTop: function() {
                        try {
                            let invocation = null;
                            let match = null;
                            let enable = null;
                            if (arguments.length >= 1 && typeof arguments[0] === 'object' && arguments[0] !== null && typeof arguments[0].return_value === 'function') {
                                // invocation-first form
                                invocation = arguments[0];
                                match = arguments[1];
                                enable = arguments[2];
                            } else {
                                // simple params form (match, enable)
                                match = arguments[0];
                                enable = arguments[1];
                            }

                            log('OpenBoardAlwaysOnTop: SetAlwaysOnTop called raw match="' + match + '" enable=' + enable + ' (invocation=' + (invocation ? 'yes' : 'no') + ')');
                            // normalize arguments: some callers may pass boolean first (enable only) or strings 'true'/'false'
                            if ((typeof match === 'string') && (match === 'true' || match === 'false')) {
                                // caller sent boolean as string; convert
                                match = (match === 'true');
                            }
                            if ((typeof match === 'boolean') && (enable === undefined || enable === null)) {
                                // caller passed only a boolean -> interpret as enable only
                                log('OpenBoardAlwaysOnTop: caller passed boolean as first arg; using default match string');
                                enable = match;
                                match = 'OpenBoard Desktop Palette';
                            }
                            // ensure enable is boolean if it's a string
                            if ((typeof enable === 'string') && (enable === 'true' || enable === 'false')) {
                                enable = (enable === 'true');
                            }
                            log('OpenBoardAlwaysOnTop: normalized match="' + match + '" enable=' + enable);
                            let actors = global.get_window_actors();
                            log('OpenBoardAlwaysOnTop: window_actors_count=' + actors.length);
                            let found = false;
                            for (let i = 0; i < actors.length; ++i) {
                                let a = actors[i];
                                let w = a.meta_window;
                                if (!w) { log('OpenBoardAlwaysOnTop: actor['+i+'] has no meta_window'); continue; }
                                let title = '';
                                try { title = w.get_title ? w.get_title() : (w.title || ''); } catch (e) { title = ''; }
                                let wmclass = '';
                                try { wmclass = w.get_wm_class ? w.get_wm_class() : ''; } catch (e) { wmclass = ''; }
                                log('OpenBoardAlwaysOnTop: checking actor['+i+'] title="'+title+'" wmclass="'+wmclass+'"');
                                let matches = false;
                                try { matches = ((title && match && title.indexOf(match) !== -1) || (wmclass && match && wmclass.indexOf(match) !== -1)); } catch (e) { matches = false; }
                                log('OpenBoardAlwaysOnTop: actor['+i+'] matches=' + matches);
                                if (matches) {
                                    found = true;
                                    if (enable) {
                                        try { if (w.make_above) w.make_above(); } catch (e) { log('make_above error: ' + e); }
                                        try { if (w.set_keep_above) w.set_keep_above(true); } catch (e) { log('set_keep_above error: ' + e); }
                                        log('OpenBoardAlwaysOnTop: made window above for actor['+i+']');
                                    } else {
                                        try { if (w.unmake_above) w.unmake_above(); } catch (e) { log('unmake_above error: ' + e); }
                                        try { if (w.set_keep_above) w.set_keep_above(false); } catch (e) { log('set_keep_above(false) error: ' + e); }
                                        log('OpenBoardAlwaysOnTop: unset keep above for actor['+i+']');
                                    }
                                }
                            }

                            log('OpenBoardAlwaysOnTop: SetAlwaysOnTop finished found=' + found + ', returning');
                            let ret = new GLib.Variant('(b)', [found]);
                            // helper to return via invocation (GDBusMethodInvocation in GJS may expose returnValue or return_value)
                            function _invocationReturn(inv, value) {
                                try {
                                    if (!inv) return false;
                                    if (typeof inv.returnValue === 'function') { inv.returnValue(value); return true; }
                                    if (typeof inv.return_value === 'function') { inv.return_value(value); return true; }
                                    // fallback: try calling a method named 'return' (unlikely)
                                    if (typeof inv.return === 'function') { inv.return(value); return true; }
                                    log('OpenBoardAlwaysOnTop: invocation has no known return method; keys=' + JSON.stringify(Object.getOwnPropertyNames(inv)));
                                } catch (e) { log('OpenBoardAlwaysOnTop: _invocationReturn error: ' + e); }
                                return false;
                            }

                            if (invocation) {
                                if (_invocationReturn(invocation, ret)) return;
                            }
                            return ret;
                        } catch (e) {
                            log('OpenBoardAlwaysOnTop SetAlwaysOnTop error: ' + e);
                            try {
                                let ret2 = new GLib.Variant('(b)', [false]);
                                if (arguments.length >= 1 && typeof arguments[0] === 'object' && arguments[0] !== null) {
                                    try {
                                        // try both naming conventions
                                        if (typeof arguments[0].returnValue === 'function') { arguments[0].returnValue(ret2); return; }
                                        if (typeof arguments[0].return_value === 'function') { arguments[0].return_value(ret2); return; }
                                        try { if (typeof arguments[0].return === 'function') { arguments[0].return(ret2); return; } } catch(e) {}
                                        log('OpenBoardAlwaysOnTop: could not call return on invocation object; keys=' + JSON.stringify(Object.getOwnPropertyNames(arguments[0])));
                                    } catch (e2) { log('return_value failed: ' + e2); }
                                    return;
                                }
                                return ret2;
                            } catch (e2) { log('return_value failed: ' + e2); }
                        }
                    }
                };

                this._exported = Gio.DBusExportedObject.wrapJSObject(ifaceInfo, serviceObj);
                log('OpenBoardAlwaysOnTop: wrapped exported object');

                this._ownerId = Gio.bus_own_name(Gio.BusType.SESSION,
                    'org.gnome.Shell.Extensions.OpenBoard',
                    Gio.BusNameOwnerFlags.NONE,
                    (connection, name) => {
                        try {
                            log('OpenBoardAlwaysOnTop: bus_own_name acquired callback name=' + name);
                            this._connection = connection;
                            if (this._exported) {
                                this._exported.export(connection, '/org/gnome/Shell/Extensions/OpenBoard');
                                log('OpenBoardAlwaysOnTop: exported DBus object on ' + name + ' at /org/gnome/Shell/Extensions/OpenBoard');
                            } else {
                                log('OpenBoardAlwaysOnTop: _exported is null in owner callback');
                            }
                        } catch (e) {
                            log('OpenBoardAlwaysOnTop: export failed in owner callback: ' + e);
                        }
                    },
                    (connection, name) => {
                        log('OpenBoardAlwaysOnTop: bus_own_name lost name=' + name);
                        // connection lost
                        try { this._connection = null; } catch (e) {}
                    },
                    null);

                log('OpenBoardAlwaysOnTop: requested bus name, ownerId=' + this._ownerId);
            } catch (e) {
                log('OpenBoardAlwaysOnTop: DBus setup failed: ' + e);
            }

        } catch (e) {
            log('OpenBoardAlwaysOnTop enable error: ' + e);
        }
    }

    disable() {
        try {
            log('OpenBoardAlwaysOnTop: disable (class)');
            if (this._indicator) {
                this._indicator.destroy();
                this._indicator = null;
            }

            // Unexport DBus object and release name
            try {
                if (this._ownerId) {
                    try { Gio.bus_unown_name(this._ownerId); } catch (e) { log('bus_unown_name error: ' + e); }
                    this._ownerId = 0;
                }
                if (this._exported) {
                    try { this._exported.unexport(); } catch (e) { log('exported.unexport error: ' + e); }
                    this._exported = null;
                }
            } catch (e) {
                log('OpenBoardAlwaysOnTop disable DBus cleanup error: ' + e);
            }

        } catch (e) {
            log('OpenBoardAlwaysOnTop disable error: ' + e);
        }
    }
}