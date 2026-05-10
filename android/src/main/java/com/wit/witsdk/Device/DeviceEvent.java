package com.wit.witsdk.Device;

import android.bluetooth.BluetoothDevice;

import com.wit.witsdk.Device.Interface.DeviceDataListener;
import com.wit.witsdk.Device.Interface.DeviceFindListener;

import java.util.ArrayList;
import java.util.List;

public class DeviceEvent {
    private List<DeviceFindListener> findListeners = new ArrayList<>();
    private List<DeviceDataListener> deviceListeners = new ArrayList<>();

    public void AddDeviceFindListener(DeviceFindListener listener) {
        findListeners.add(listener);
    }

    public void RemoveDeviceFindListener(DeviceFindListener listener) {
        findListeners.remove(listener);
    }

    public void FindDevice(BluetoothDevice device) {
        for (DeviceFindListener listener : findListeners) {
            listener.onDeviceFound(device);
        }
    }

    public void AddDeviceListener(DeviceDataListener listener) {
        deviceListeners.add(listener);
    }

    public void RemoveDeviceListener(DeviceDataListener listener) {
        deviceListeners.remove(listener);
    }

    public void OnReceiveDevice(String deviceName, String displayData) {
        for (DeviceDataListener listener : deviceListeners) {
            listener.OnReceive(deviceName, displayData);
        }
    }

    public void OnStatusChange(String deviceName, boolean status){
        for (DeviceDataListener listener : deviceListeners) {
            listener.OnStatusChange(deviceName, status);
        }
    }
}
