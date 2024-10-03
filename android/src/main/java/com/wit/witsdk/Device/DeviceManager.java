package com.wit.witsdk.Device;

import java.util.Objects;
import java.util.concurrent.ConcurrentHashMap;
import java.util.concurrent.ConcurrentMap;

public class DeviceManager extends DeviceEvent{
    ConcurrentMap<String, DeviceModel> deviceMap = new ConcurrentHashMap<String, DeviceModel>();
    private static DeviceManager instance;
    private DeviceManager() {}

    public static synchronized DeviceManager getInstance() {
        if (instance == null) {
            instance = new DeviceManager();
        }
        return instance;
    }

    public void AddDevice(String key, DeviceModel deviceModel){
        deviceMap.put(key, deviceModel);
    }

    public void RemoveDevice(String key){
        deviceMap.remove(key);
    }

    public void CleanAllDevice(){
        for (String key : deviceMap.keySet()) {
            Objects.requireNonNull(deviceMap.get(key)).CloseDevice();
        }
        deviceMap.clear();
    }

    public DeviceModel GetDevice(String key){
        return deviceMap.getOrDefault(key, null);
    }
}
