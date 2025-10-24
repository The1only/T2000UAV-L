package com.wit.witsdk.Device;

import android.annotation.SuppressLint;
import android.bluetooth.BluetoothDevice;
import android.bluetooth.BluetoothGatt;
import android.bluetooth.BluetoothGattCallback;
import android.bluetooth.BluetoothGattCharacteristic;
import android.bluetooth.BluetoothGattDescriptor;
import android.bluetooth.BluetoothGattService;
import android.bluetooth.BluetoothProfile;
import android.content.Context;
import android.os.Handler;
import android.util.Log;

import java.text.SimpleDateFormat;
import java.util.ArrayList;
import java.util.Date;
import java.util.List;
import java.util.UUID;
import java.util.concurrent.ConcurrentHashMap;
import java.util.concurrent.ConcurrentMap;

public class DeviceModel {
    private static final String TAG = "WitLOG";
    public String deviceName;
    public ConcurrentMap<String, Double> deviceData = new ConcurrentHashMap<String, Double>();
    private DeviceManager deviceManager = DeviceManager.getInstance();
    private List<Byte> activeByteTemp = new ArrayList<>();
    public boolean isConnect = false;
    private BluetoothDevice device;
    private BluetoothGatt bluetoothGatt;
    private BluetoothGattCharacteristic writeCharacteristic;

    private final String SERVICE_UUID = "0000ffe5-0000-1000-8000-00805f9a34fb";
    private final String READ_UUID = "0000ffe4-0000-1000-8000-00805f9a34fb";
    private final String WRITE_UUID = "0000ffe9-0000-1000-8000-00805f9a34fb";

    public DeviceModel(String name, BluetoothDevice device) {
        this.deviceName = name;
        this.device = device;
    }

    @SuppressLint("MissingPermission")
    public void Connect(Context ctx) throws Exception{
        if(isConnect){
            return;
        }

        BluetoothGattCallback gattCallback = new BluetoothGattCallback() {
            @Override
            public void onConnectionStateChange(BluetoothGatt gatt, int status, int newState) {
                super.onConnectionStateChange(gatt, status, newState);
                if (newState == BluetoothProfile.STATE_CONNECTED) {
                    gatt.discoverServices();
                    deviceManager.OnStatusChange(deviceName, true);
                    Log.i(TAG, deviceName + "is now connected");
                } else if (newState == BluetoothProfile.STATE_DISCONNECTED) {
                    gatt.close();
                    bluetoothGatt = null;
                    deviceManager.OnStatusChange(deviceName, false);
                    Log.i(TAG, deviceName + "is now disconnected");
                }
            }

            @Override
            public void onServicesDiscovered(BluetoothGatt gatt, int status) {
                if (status == BluetoothGatt.GATT_SUCCESS) {
                    Log.i(TAG, "GATT_SUCCESS");
                    for (BluetoothGattService bluetoothGattService : gatt.getServices()) {
                        // Service...
                        Log.i(TAG, "Service_UUID" + bluetoothGattService.getUuid());
                        if (SERVICE_UUID.equals(bluetoothGattService.getUuid().toString())) {
                            for (BluetoothGattCharacteristic characteristic : bluetoothGattService.getCharacteristics()) {
                                // UUID
                                if (READ_UUID.equals(characteristic.getUuid().toString())) {
                                    Log.i(TAG, "Match to UUID_READ");
                                    gatt.setCharacteristicNotification(characteristic, true);

                                    // 对于NOTIFY或INDICATE，需要添加Descriptor
                                    BluetoothGattDescriptor descriptor = characteristic.getDescriptor(
                                            UUID.fromString("00002902-0000-1000-8000-00805f9b34fb"));
                                    if (descriptor != null) {
                                        descriptor.setValue(BluetoothGattDescriptor.ENABLE_NOTIFICATION_VALUE);
                                        gatt.writeDescriptor(descriptor);
                                    }
                                }

                                if (WRITE_UUID.equals(characteristic.getUuid().toString())) {
                                    Log.i(TAG, "Match to UUID_WRITE");
                                    bluetoothGatt = gatt;
                                    writeCharacteristic = characteristic;
                                }
                            }
                            break;
                        }
                    }
                } else {
                    Log.e(TAG, "onServicesDiscovered received: " + status);
                }
            }

            @Override
            public void onCharacteristicChanged(BluetoothGatt gatt, BluetoothGattCharacteristic characteristic) {
                // readUUID。
                if (READ_UUID.equals(characteristic.getUuid().toString())) {
                    OnReceiveBle(characteristic.getValue());
                }
            }
        };

        try {
            device.connectGatt(ctx, false, gattCallback);
            isConnect = true;

            new Thread(new Runnable() {
                @Override
                public void run() {
                    ReadDataTh();
                }
            }).start();
        }catch (Exception e){
            Log.e(TAG, "connectGatt error ", e);
        }
    }

    @SuppressLint("MissingPermission")
    public void CloseDevice(){
        try {
            if(bluetoothGatt != null){
                bluetoothGatt.disconnect();
            }
        }catch (Exception e){
            Log.e(TAG, deviceName + "CloseDevice error ", e);
        }finally {
            isConnect = false;
        }
    }

    private void ReadDataTh() {
        while (isConnect){
            try {
                if(bluetoothGatt != null || writeCharacteristic != null)
                {
                    // Get Magnetic field...
                    SendData(new byte[]{ (byte)0xff, (byte)0xaa, (byte)0x27, (byte)0x3a, (byte)0x00},false);
//                    Thread.sleep(500);
                 //   Thread.sleep(20);

                    // Get power...
                //    SendData(new byte[]{ (byte)0xff, (byte)0xaa, (byte)0x27, (byte)0x64, (byte)0x00},false);
                //    Thread.sleep(20);
//                    Thread.sleep(500);
                    // Get Temperature...
                //    SendData(new byte[]{ (byte)0xff, (byte)0xaa, (byte)0x27, (byte)0x40, (byte)0x00},false);
                }
                Thread.sleep(20);
//                Thread.sleep(500);
            }
            catch (Exception ex){
                Log.e(TAG, "ReadDataTh error", ex);
            }
        }
    }

    public void OnReceiveBle(byte[] value){
        List<Byte> dataBuffer = new ArrayList<>();
        for (byte b : value) {
            dataBuffer.add(b);
        }

        // 不是55 61 或者 55 71
        while (dataBuffer.size() > 2 && (dataBuffer.get(0) != 0x55 || (dataBuffer.get(1) != 0x61 && dataBuffer.get(1) != 0x71))){
            dataBuffer.remove(0);
        }

        while (dataBuffer.size() >= 20){
            activeByteTemp = new ArrayList<>(dataBuffer.subList(0,20));
            dataBuffer = new ArrayList<>(dataBuffer.subList(20, dataBuffer.size()));
            if(activeByteTemp.get(0) == 0x55 && activeByteTemp.get(1) == 0x61){
                double acc_x = GetShortData(activeByteTemp.get(2), activeByteTemp.get(3)) / 32768.0 * 16;
                double acc_y = GetShortData(activeByteTemp.get(4), activeByteTemp.get(5)) / 32768.0 * 16;
                double acc_z = GetShortData(activeByteTemp.get(6), activeByteTemp.get(7)) / 32768.0 * 16;

                double as_x = GetShortData(activeByteTemp.get(8), activeByteTemp.get(9)) / 32768.0 * 2000;
                double as_y = GetShortData(activeByteTemp.get(10), activeByteTemp.get(11)) / 32768.0 * 2000;
                double as_z = GetShortData(activeByteTemp.get(12), activeByteTemp.get(13)) / 32768.0 * 2000;

                double angle_x = GetShortData(activeByteTemp.get(14), activeByteTemp.get(15)) / 32768.0 * 180;
                double angle_y = GetShortData(activeByteTemp.get(16), activeByteTemp.get(17)) / 32768.0 * 180;
                double angle_z = GetShortData(activeByteTemp.get(18), activeByteTemp.get(19)) / 32768.0 * 180;

                SetData("AccX", Math.round(acc_x * 1000.0) / 1000.0);
                SetData("AccY", Math.round(acc_y * 1000.0) / 1000.0);
                SetData("AccZ", Math.round(acc_z * 1000.0) / 1000.0);
                SetData("AsX", Math.round(as_x * 1000.0) / 1000.0);
                SetData("AsY", Math.round(as_y * 1000.0) / 1000.0);
                SetData("AsZ", Math.round(as_z * 1000.0) / 1000.0);
                SetData("AngX", Math.round(angle_x * 100.0) / 100.0);
                SetData("AngY", Math.round(angle_y * 100.0) / 100.0);
                SetData("AngZ", Math.round(angle_z * 100.0) / 100.0);

                deviceManager.OnReceiveDevice(deviceName, GetDataDisplayLine());

            }
            else if(activeByteTemp.get(0) == 0x55 && activeByteTemp.get(1) == 0x71){
                if(activeByteTemp.get(2) == 58){
                    double h_x = GetShortData(activeByteTemp.get(4), activeByteTemp.get(5)) / 120.0;
                    double h_y = GetShortData(activeByteTemp.get(6), activeByteTemp.get(7)) / 120.0;
                    double h_z = GetShortData(activeByteTemp.get(8), activeByteTemp.get(9)) / 120.0;
                    SetData("HX", Math.round(h_x * 1000.0) / 1000.0);
                    SetData("HY", Math.round(h_y * 1000.0) / 1000.0);
                    SetData("HZ", Math.round(h_z * 1000.0) / 1000.0);
                }
                else if(activeByteTemp.get(2) == 0x64){
                    double e = GetShortData(activeByteTemp.get(4), activeByteTemp.get(5)) / 100.0;
                    SetData("Electricity", GetBatteryPercent(e));
                }
                else if(activeByteTemp.get(2) == 0x40){
                    double e = GetShortData(activeByteTemp.get(4), activeByteTemp.get(5)) / 100.0;
                    SetData("Temperature", e);
                }
            }
            else{
                Log.e(TAG," A: "+activeByteTemp.get(0)+"  B: "+activeByteTemp.get(1) );
            }
        }
    }

    private double GetBatteryPercent(double v) {
        if (v > 3.96) {
            return 100;
        } else if (v > 3.93) {
            return 90;
        } else if (v > 3.87) {
            return 75;
        } else if (v > 3.82) {
            return 60;
        } else if (v > 3.79) {
            return 50;
        } else if (v > 3.77) {
            return 40;
        } else if (v > 3.73) {
            return 30;
        } else if (v > 3.70) {
            return 20;
        } else if (v > 3.68) {
            return 15;
        } else if (v > 3.50) {
            return 10;
        } else if (v > 3.40) {
            return 5;
        } else {
            return 0.0;
        }
    }

    private short GetShortData(byte low, byte high){
        return (short) ((((short) high) << 8) | ((short) low & 0xff));
    }

    public void SetData(String key, double data){
        deviceData.put(key, data);
    }

    public Double GetData(String key){
        return deviceData.getOrDefault(key, 0.0);
    }

    public String GetDataDisplay(){
        String data = "AccX: " + GetData("AccX").toString() + "\r\n";
        data += "AccY: " + GetData("AccY").toString() + "\r\n";
        data += "AccZ: " + GetData("AccZ").toString() + "\r\n";
        data += "AsX: " + GetData("AsX").toString() + "\r\n";
        data += "AsY: " + GetData("AsY").toString() + "\r\n";
        data += "AsZ: " + GetData("AsZ").toString() + "\r\n";
        data += "AngleX: " + GetData("AngX").toString() + "\r\n";
        data += "AngleY: " + GetData("AngY").toString() + "\r\n";
        data += "AngleZ: " + GetData("AngZ").toString() + "\r\n";
        data += "HX: " + GetData("HX").toString() + "\r\n";
        data += "HY: " + GetData("HY").toString() + "\r\n";
        data += "HZ: " + GetData("HZ").toString() + "\r\n";
        data += "Electricity: " + GetData("Electricity").toString() + "\r\n";
        data += "Temperature: " + GetData("Temperature").toString();
        return data;
    }

    @SuppressLint("DefaultLocale")
    public String GetDataDisplayLine(){
        String data = "AccX " + String.format("%.3f", GetData("AccX")) + "\t";
        data += "AccY " + String.format("%.3f", GetData("AccY")) + "\t";
        data += "AccZ " + String.format("%.3f", GetData("AccZ")) + "\t";
        data += "AsX " + String.format("%.3f", GetData("AsX")) + "\t";
        data += "AsY " + String.format("%.3f", GetData("AsY")) + "\t";
        data += "AsZ " + String.format("%.3f", GetData("AsZ")) + "\t";
        data += "AngleX " + String.format("%.2f", GetData("AngX")) + "\t";
        data += "AngleY " + String.format("%.2f", GetData("AngY")) + "\t";
        data += "AngleZ " + String.format("%.2f", GetData("AngZ")) + "\t";
        data += "HX " + GetData("HX").toString() + "\t";
        data += "HY " + GetData("HY").toString() + "\t";
        data += "HZ " + GetData("HZ").toString() + "\t";
        data += "Electricity " + GetData("Electricity").toString()+ "\t";
        data += "Temperature " + GetData("Temperature").toString()+ "\n";
        return data;
    }

    public void ReSet(){
        byte[] data = new byte[] { (byte)0xff, (byte)0xaa, (byte)0x00, (byte)0x01, (byte)0x00};
        SendData(data, false);
    }

    public void SetRRate(int input){
        int value = 0;
        switch (input){
            case 1:
                value = 3;
                break;
            case 5:
                value = 5;
                break;
            case 10:
                value = 6;
                break;
            case 50:
                value = 8;
                break;
            case 100:
                value = 9;
                break;
            case 200:
                value = 11;
                break;
            default:
                return;
        }
        byte[] data = new byte[] { (byte)0xff, (byte)0xaa, (byte)0x03, (byte) value, 0};
        SendData(data, true);
    }

    public void SetBandwidth(int input){
        int value = 0;
        switch (input){
            case 5:
                value = 7;
                break;
            case 10:
                value = 6;
                break;
            case 20:
                value = 5;
                break;
            case 42:
                value = 4;
                break;
            case 98:
                value = 3;
                break;
            case 188:
                value = 2;
                break;
            default:
                return;
        }
        byte[] data = new byte[] { (byte)0xff, (byte)0xaa, (byte)0x1f, (byte)value, 0};
        SendData(data, true);
    }

    public void SetAngle0(){
        byte[] data = new byte[] { (byte)0xff, (byte)0xaa, (byte)0x01, (byte)0x08, (byte)0x00};
        SendData(data, false);
    }

    public void SetMagStart(){
        byte[] data = new byte[] { (byte)0xff, (byte)0xaa, (byte)0x01, (byte)0x07, (byte)0x00};
        SendData(data, false);
    }

    public void SetMagStop(){
        byte[] data = new byte[] { (byte)0xff, (byte)0xaa, (byte)0x01, (byte)0x00, (byte)0x00};
        SendData(data, true);
    }

    @SuppressLint("MissingPermission")
    private void SendData(byte[] data, boolean isSave){
        new Thread(new Runnable() {
            @Override
            public void run() {
                try {
                    if(bluetoothGatt == null || writeCharacteristic == null){
                        return;
                    }
                    writeCharacteristic.setValue(data);
                    boolean writeResult = bluetoothGatt.writeCharacteristic(writeCharacteristic);
                    if(isSave){
                        // 100ms
                        new Handler().postDelayed(new Runnable() {
                            @Override
                            public void run() {
                                writeCharacteristic.setValue(new byte[] { (byte)0xff, (byte)0xaa, (byte)0x00, (byte)0x00, (byte)0x00});
                                boolean writeResult2 = bluetoothGatt.writeCharacteristic(writeCharacteristic);
                                if (writeResult2) {
                                    Log.i(TAG, "Successfully sent and saved");
                                }
                            }
                        }, 100);

                    }
                }catch (Exception e){
                    Log.e(TAG, "Sending data error！", e);
                }
            }
        }).start();
    }
}
