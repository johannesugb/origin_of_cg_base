﻿//------------------------------------------------------------------------------
// <auto-generated>
//     This code was generated by a tool.
//     Runtime Version:4.0.30319.42000
//
//     Changes to this file may cause incorrect behavior and will be lost if
//     the code is regenerated.
// </auto-generated>
//------------------------------------------------------------------------------

namespace CgbPostBuildHelper.Properties {
    
    
    [global::System.Runtime.CompilerServices.CompilerGeneratedAttribute()]
    [global::System.CodeDom.Compiler.GeneratedCodeAttribute("Microsoft.VisualStudio.Editors.SettingsDesigner.SettingsSingleFileGenerator", "16.0.0.0")]
    internal sealed partial class Settings : global::System.Configuration.ApplicationSettingsBase {
        
        private static Settings defaultInstance = ((Settings)(global::System.Configuration.ApplicationSettingsBase.Synchronized(new Settings())));
        
        public static Settings Default {
            get {
                return defaultInstance;
            }
        }
        
        [global::System.Configuration.UserScopedSettingAttribute()]
        [global::System.Diagnostics.DebuggerNonUserCodeAttribute()]
        [global::System.Configuration.DefaultSettingValueAttribute("False")]
        public bool AlwaysDeployReleaseDlls {
            get {
                return ((bool)(this["AlwaysDeployReleaseDlls"]));
            }
            set {
                this["AlwaysDeployReleaseDlls"] = value;
            }
        }
        
        [global::System.Configuration.UserScopedSettingAttribute()]
        [global::System.Diagnostics.DebuggerNonUserCodeAttribute()]
        [global::System.Configuration.DefaultSettingValueAttribute("False")]
        public bool AlwaysCopyNeverSymlink {
            get {
                return ((bool)(this["AlwaysCopyNeverSymlink"]));
            }
            set {
                this["AlwaysCopyNeverSymlink"] = value;
            }
        }
        
        [global::System.Configuration.UserScopedSettingAttribute()]
        [global::System.Diagnostics.DebuggerNonUserCodeAttribute()]
        [global::System.Configuration.DefaultSettingValueAttribute("False")]
        public bool DoNotMonitorFiles {
            get {
                return ((bool)(this["DoNotMonitorFiles"]));
            }
            set {
                this["DoNotMonitorFiles"] = value;
            }
        }
        
        [global::System.Configuration.UserScopedSettingAttribute()]
        [global::System.Diagnostics.DebuggerNonUserCodeAttribute()]
        [global::System.Configuration.DefaultSettingValueAttribute("False")]
        public bool ShowWindowForVkShaderDeployment {
            get {
                return ((bool)(this["ShowWindowForVkShaderDeployment"]));
            }
            set {
                this["ShowWindowForVkShaderDeployment"] = value;
            }
        }
        
        [global::System.Configuration.UserScopedSettingAttribute()]
        [global::System.Diagnostics.DebuggerNonUserCodeAttribute()]
        [global::System.Configuration.DefaultSettingValueAttribute("False")]
        public bool ShowWindowForGlShaderDeployment {
            get {
                return ((bool)(this["ShowWindowForGlShaderDeployment"]));
            }
            set {
                this["ShowWindowForGlShaderDeployment"] = value;
            }
        }
        
        [global::System.Configuration.UserScopedSettingAttribute()]
        [global::System.Diagnostics.DebuggerNonUserCodeAttribute()]
        [global::System.Configuration.DefaultSettingValueAttribute("False")]
        public bool ShowWindowForModelDeployment {
            get {
                return ((bool)(this["ShowWindowForModelDeployment"]));
            }
            set {
                this["ShowWindowForModelDeployment"] = value;
            }
        }
        
        [global::System.Configuration.UserScopedSettingAttribute()]
        [global::System.Diagnostics.DebuggerNonUserCodeAttribute()]
        [global::System.Configuration.DefaultSettingValueAttribute("release")]
        public string ReleaseSubPathInExternals {
            get {
                return ((string)(this["ReleaseSubPathInExternals"]));
            }
            set {
                this["ReleaseSubPathInExternals"] = value;
            }
        }
        
        [global::System.Configuration.UserScopedSettingAttribute()]
        [global::System.Diagnostics.DebuggerNonUserCodeAttribute()]
        [global::System.Configuration.DefaultSettingValueAttribute("debug")]
        public string DebugSubPathInExternals {
            get {
                return ((string)(this["DebugSubPathInExternals"]));
            }
            set {
                this["DebugSubPathInExternals"] = value;
            }
        }
    }
}
