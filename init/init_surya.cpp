/*
   Copyright (c) 2015, The Linux Foundation. All rights reserved.
   Copyright (C) 2016 The CyanogenMod Project.
   Copyright (C) 2019-2020 The LineageOS Project.

   Redistribution and use in source and binary forms, with or without
   modification, are permitted provided that the following conditions are
   met:
    * Redistributions of source code must retain the above copyright
      notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above
      copyright notice, this list of conditions and the following
      disclaimer in the documentation and/or other materials provided
      with the distribution.
    * Neither the name of The Linux Foundation nor the names of its
      contributors may be used to endorse or promote products derived
      from this software without specific prior written permission.
   THIS SOFTWARE IS PROVIDED "AS IS" AND ANY EXPRESS OR IMPLIED
   WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
   MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NON-INFRINGEMENT
   ARE DISCLAIMED.  IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS
   BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
   CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
   SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR
   BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
   WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE
   OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN
   IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include <cstdlib>
#include <fstream>
#include <string.h>
#include <sys/sysinfo.h>
#include <unistd.h>
#include <vector>

#include <android-base/properties.h>
#define _REALLY_INCLUDE_SYS__SYSTEM_PROPERTIES_H_
#include <sys/_system_properties.h>

#include "property_service.h"
#include "vendor_init.h"

using android::base::GetProperty;
using android::base::SetProperty;

char const *heapstartsize;
char const *heapgrowthlimit;
char const *heapsize;
char const *heapminfree;
char const *heapmaxfree;
char const *heaptargetutilization;

std::vector<std::string> ro_props_default_source_order = {
    "",
    "odm.",
    "product.",
    "system.",
    "vendor.",
};

void check_device()
{
    struct sysinfo sys;

    sysinfo(&sys);

    if (sys.totalram >= 5ull * 1024 * 1024 * 1024){
        // from - phone-xhdpi-6144-dalvik-heap.mk
        heapstartsize = "16m";
        heapgrowthlimit = "256m";
        heapsize = "512m";
        heaptargetutilization = "0.5";
        heapminfree = "8m";
        heapmaxfree = "32m";
    } else if (sys.totalram >= 7ull * 1024 * 1024 * 1024) {
        // from - phone-xhdpi-8192-dalvik-heap.mk
        heapstartsize = "24m";
        heapgrowthlimit = "256m";
        heapsize = "512m";
        heaptargetutilization = "0.46";
        heapminfree = "8m";
        heapmaxfree = "48m";
    }
}

void property_override(char const prop[], char const value[], bool add = true) {
    prop_info *pi;

    pi = (prop_info *)__system_property_find(prop);
    if (pi)
        __system_property_update(pi, value, strlen(value));
    else if (add)
        __system_property_add(prop, strlen(prop), value, strlen(value));
}

void set_device_props(const std::string fingerprint, const std::string description,
        const std::string brand, const std::string device, const std::string model) {
    const auto set_ro_build_prop = [](const std::string &source,
                                      const std::string &prop,
                                      const std::string &value) {
        auto prop_name = "ro." + source + "build." + prop;
        property_override(prop_name.c_str(), value.c_str(), false);
    };

    const auto set_ro_product_prop = [](const std::string &source,
                                        const std::string &prop,
                                        const std::string &value) {
        auto prop_name = "ro.product." + source + prop;
        property_override(prop_name.c_str(), value.c_str(), false);
    };

    for (const auto &source : ro_props_default_source_order) {
        set_ro_build_prop(source, "fingerprint", fingerprint);
        set_ro_product_prop(source, "brand", brand);
        set_ro_product_prop(source, "device", device);
        set_ro_product_prop(source, "model", model);
    }

    property_override("ro.build.fingerprint", fingerprint.c_str());
    property_override("ro.build.description", description.c_str());
    property_override("ro.bootimage.build.fingerprint", fingerprint.c_str());
    property_override("ro.system_ext.build.fingerprint", fingerprint.c_str());
    property_override("ro.com.google.clientidbase", "android-xiaomi");
    property_override("ro.com.google.clientidbase.ax", "android-xiaomi-rvo3");
    property_override("ro.com.google.clientidbase.ms", "android-xiaomi-rvo3");
    property_override("ro.com.google.clientidbase.tx", "android-xiaomi-rvo3");
    property_override("ro.com.google.clientidbase.vs", "android-xiaomi-rvo3");

    property_override("ro.control_privapp_permissions", "log");
}

void vendor_load_properties() {
    std::string hwname = GetProperty("ro.boot.hwname", "");
    char const fp[] = "Xiaomi/dipper/dipper:8.1.0/OPM1.171019.011/V9.5.5.0.OEAMIFA:user/release-keys";
    char const fp_desc[] = "dipper-user 8.1.0 OPM1.171019.011 V9.5.5.0.OEAMIFA release-keys";
    char nfc_prop[] = "ro.hw.nfc";

    if (hwname == "surya") {
        set_device_props(
                fp,
                fp_desc,
                "POCO", "surya", "M2007J20CG");
        property_override(nfc_prop, "1");
    } else if (hwname == "karna") {
        set_device_props(
                fp,
                fp_desc,
                "POCO", "karna", "M2007J20CI");
        property_override(nfc_prop, "0");
    }

    check_device();
    SetProperty("dalvik.vm.heapstartsize", heapstartsize);
    SetProperty("dalvik.vm.heapgrowthlimit", heapgrowthlimit);
    SetProperty("dalvik.vm.heapsize", heapsize);
    SetProperty("dalvik.vm.heaptargetutilization", heaptargetutilization);
    SetProperty("dalvik.vm.heapminfree", heapminfree);
    SetProperty("dalvik.vm.heapmaxfree", heapmaxfree);

    //Safetynet workarounds
    property_override("ro.oem_unlock_supported", "0");
    property_override("ro.boot.verifiedbootstate", "green");
}
