package com.wit.witsdk.Device;

import androidx.appcompat.app.AppCompatActivity;

import android.annotation.SuppressLint;
import android.bluetooth.BluetoothDevice;
import android.bluetooth.le.ScanCallback;
import android.bluetooth.le.ScanResult;

import android.content.Intent;
import android.os.Bundle;
import android.util.Log;

import android.view.View;
import android.view.WindowManager;

import android.widget.AdapterView;
import android.widget.ListView;
import android.widget.Switch;
import android.widget.Toast;
import android.content.Context;
import android.app.Activity;

import com.wit.example.UI.CustomAdapter;
import com.wit.example.UI.ListItem;
import com.wit.witsdk.Bluetooth.WitBluetoothManager;
import com.wit.witsdk.Device.DeviceManager;
import com.wit.witsdk.Device.DeviceModel;
import com.wit.witsdk.Device.Interface.DeviceDataListener;
import com.wit.witsdk.Device.Interface.DeviceFindListener;

import java.util.ArrayList;
import java.util.List;
import java.util.Objects;
import java.util.Timer;
import java.util.TimerTask;

public class IMUActivity implements DeviceDataListener, DeviceFindListener
{
    private static final String TAG = "WitLOG";
    private Timer timer;
    private Context mcontext;
    private Activity mactivity;
    private final List<ListItem> findList = new ArrayList<>();

    private CustomAdapter customAdapter;
    private final DeviceManager deviceManager = DeviceManager.getInstance();

    private boolean active = false;
    private String latestIMU = "";
    private String lastDevice = "";

    private static void status(String str) {
//        System.out.println(str);
        Log.i(TAG, str);
    }

    public String TestIMU()
    {
        String ret = "false";
        if(active) ret = "true";
        status("********* TestIMU  ****" + ret);
        return ret;
    }

    public String getIMU()
    {
        return latestIMU;
    }

    public IMUActivity(Context contextIn)
    {
        mcontext = contextIn;
        mactivity = (Activity) mcontext;

        // Apply for Bluetooth and location permissions
        WitBluetoothManager.requestPermissions(mactivity);

        // Monitoring device events
        deviceManager.AddDeviceListener(this);
        deviceManager.AddDeviceFindListener(this);

        // Search switch
        StartScan();
    }

    /**
     * Start searching for Bluetooth
     * */
    private void StartScan(){
        active = false;
        DeviceManager.getInstance().CleanAllDevice();

        try {
            WitBluetoothManager witBluetoothManager = WitBluetoothManager.getInstance(mcontext);
            witBluetoothManager.startScan();
        } catch (Exception e) {
            status("Start searching for anomalies：" + e.getMessage());
        }
    }

    /**
     * End search for Bluetooth
     * */
    private void StopScan(){
        try {
            WitBluetoothManager witBluetoothManager = WitBluetoothManager.getInstance(mactivity);
            witBluetoothManager.stopScan();
        } catch (Exception e) {
            Log.e(TAG, "Search Error："+ e.getMessage());
        }
    }

    /**
     * Call back this method when the device is found
     * */
    @SuppressLint("MissingPermission")
    @Override
    public void onDeviceFound(BluetoothDevice device) {
        String deviceName = device.getName();
        if(deviceName != null && deviceName.startsWith("WT") && active == false){
            status(deviceName);
            String name = deviceName + "(" + device.getAddress() +")";
            DeviceModel deviceModel = new DeviceModel(name, device);
            deviceManager.AddDevice(name, deviceModel);
            try {
                deviceModel.Connect(mcontext); //IMUActivity.this);
            } catch (Exception e) {
                Log.e(TAG, "Connect Error：" + e.getMessage());
            }
        }
    }

    /**
     * Real time data callback for devices
     * */
    @Override
    public void OnReceive(String deviceName, String displayData) {
 //       status("From: " + deviceName + " received " + displayData);
        latestIMU = displayData;
        lastDevice = deviceName;
    }

    /**
     * When the device status changes
     * */
    @Override
    public void OnStatusChange(String deviceName, boolean status) {
        if(status){
            status(deviceName + "  Connected");
            active = true;
            StopScan();
        }
        else {
            status( deviceName + "  Disconnect");
        }
    }
}
