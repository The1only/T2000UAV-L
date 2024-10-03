package com.wit.witsdk.Bluetooth;

import android.Manifest;
import android.annotation.SuppressLint;
import android.app.Activity;
import android.bluetooth.BluetoothAdapter;
import android.bluetooth.BluetoothDevice;
import android.bluetooth.le.BluetoothLeScanner;
import android.bluetooth.le.ScanCallback;
import android.bluetooth.le.ScanResult;
import android.bluetooth.le.ScanSettings;
import android.content.Context;
import android.content.Intent;
import android.content.pm.PackageManager;
import android.location.LocationManager;
import android.os.Build;
import android.os.Handler;
import android.provider.Settings;
import android.util.Log;

import androidx.core.content.ContextCompat;

import com.wit.witsdk.Device.DeviceManager;
import com.wit.witsdk.Device.DeviceModel;

import java.util.ArrayList;
import java.util.List;
import java.util.Objects;

public class WitBluetoothManager {
    // region
    private static WitBluetoothManager instance;
    private BluetoothAdapter mBtAdapter;
    private BluetoothLeScanner scanner;
    private Activity activity;
    private boolean isScan = false;
    private ScanCallback callback;
    private static final String TAG = "WitLOG";
    private static final int ACCESS_PERMISSION = 1001;
    private static final int GPS_REQUEST_CODE = 1;
    private static final int BT_REQUEST_CODE = 2;
    // endregion


    private static void status(String str) {
        System.out.println(str);
    }

    private WitBluetoothManager(Context context) {
        status("********* WitBluetoothManager  ****");
        this.activity = (Activity) context;
        mBtAdapter = getBAdapter();
    }

    private static BluetoothAdapter getBAdapter() {
        status("********* BluetoothAdapter  ****");

        BluetoothAdapter mBluetoothAdapter = null;
        try {
            mBluetoothAdapter = BluetoothAdapter.getDefaultAdapter();
        } catch (Throwable t) {
            t.printStackTrace();
        }
        return mBluetoothAdapter;
    }

    public static WitBluetoothManager getInstance(Context ctx) throws Exception {
        status("********* WitBluetoothManager  ****");

        if (!checkPermissions(ctx)) {
            throw new Exception("Bluetooth Manager is not working and lacks permissions");
        }

        if (instance == null) {
            instance = new WitBluetoothManager(ctx);
        }
        return instance;
    }

    public static void requestPermissions(Activity activity) {
        status("********* requestPermissions  ****");

        List<String> permList = new ArrayList<>();

        // 对于 Android 6.0 (API 23) 及以上版本，申请定位权限
        if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.M) {
            permList.add(Manifest.permission.ACCESS_FINE_LOCATION);
        }

        // 对于 Android 9.0 (API 28) 及以下版本，申请粗略定位权限
        if (Build.VERSION.SDK_INT <= Build.VERSION_CODES.P) {
            permList.add(Manifest.permission.ACCESS_COARSE_LOCATION);
        }

        // 对于 Android 12 (API 31) 及以上版本，申请新的蓝牙权限
        if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.S) {
            permList.add(Manifest.permission.BLUETOOTH_SCAN);
            permList.add(Manifest.permission.BLUETOOTH_CONNECT);
        }

        // 对于 Android 11 (API 30) 及以下版本，申请旧的蓝牙权限
        if (Build.VERSION.SDK_INT <= Build.VERSION_CODES.R) {
            permList.add(Manifest.permission.BLUETOOTH);
            permList.add(Manifest.permission.BLUETOOTH_ADMIN);
        }

        if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.M) {
            activity.requestPermissions(permList.toArray(new String[0]), ACCESS_PERMISSION);
        }
    }

    public static boolean checkPermissions(Context context) {
        status("********* checkPermissions  ****");

        if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.M) {
            if (ContextCompat.checkSelfPermission(context, Manifest.permission.ACCESS_FINE_LOCATION) != PackageManager.PERMISSION_GRANTED) {
                status("********1");
                return false;
            }
        }

        // 对于 Android 9.0 (API 28) 及以下版本，检查粗略定位权限
        if (Build.VERSION.SDK_INT <= Build.VERSION_CODES.P) {
            if (ContextCompat.checkSelfPermission(context, Manifest.permission.ACCESS_COARSE_LOCATION) != PackageManager.PERMISSION_GRANTED) {
                status("********2");
                return false;
            }
        }

        // 对于 Android 12 (API 31) 及以上版本，检查新的蓝牙权限
        if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.S) {
            if (ContextCompat.checkSelfPermission(context, Manifest.permission.BLUETOOTH_SCAN) != PackageManager.PERMISSION_GRANTED) {
                status("********3");
                return false;
            }
            if (ContextCompat.checkSelfPermission(context, Manifest.permission.BLUETOOTH_CONNECT) != PackageManager.PERMISSION_GRANTED) {
                status("********4");
                return false;
            }
        }

        // 对于 Android 11 (API 30) 及以下版本，检查旧的蓝牙权限
        if (Build.VERSION.SDK_INT <= Build.VERSION_CODES.R) {
            if (ContextCompat.checkSelfPermission(context, Manifest.permission.BLUETOOTH) != PackageManager.PERMISSION_GRANTED) {
                status("********5");
                return false;
            }
            if (ContextCompat.checkSelfPermission(context, Manifest.permission.BLUETOOTH_ADMIN) != PackageManager.PERMISSION_GRANTED) {
                status("********6");
                return false;
            }
        }

        return true;
    }

    @SuppressLint("MissingPermission")
    public void startScan(){
        status("********* startScan  ****");

        checkHardware();
        if(scanner == null){
            scanner = mBtAdapter.getBluetoothLeScanner();
        }

        callback = new ScanCallback() {
            @Override
            public void onScanResult(int callbackType, ScanResult result) {
                BluetoothDevice device = result.getDevice();
                DeviceManager.getInstance().FindDevice(device);
            }

            @Override
            public void onScanFailed(int errorCode) {
                super.onScanFailed(errorCode);
                Log.e(TAG, "Search Error" + errorCode);
                status("Search Error" + errorCode);
            }
        };

        ScanSettings scanSettings = new ScanSettings.Builder()
                .setScanMode(ScanSettings.SCAN_MODE_LOW_LATENCY)
                .setReportDelay(0)
                .build();
        scanner.startScan(null, scanSettings, callback);
        isScan = true;
        Log.i(TAG, "Start scanning Bluetooth ---------------");
        status("Start scanning Bluetooth ---------------");

        Handler handler = new Handler();
        handler.postDelayed(new Runnable() {
            @Override
            public void run() {
                stopScan();
            }

        }, 10000);
    }

    @SuppressLint("MissingPermission")
    public void stopScan(){
        status("********* stopScan  ****");

        if(!isScan || callback == null){
            return;
        }
        scanner.stopScan(callback);
        isScan = false;
        Log.i(TAG, "Stop scanning Bluetooth ---------------");
    }

    @SuppressLint("MissingPermission")
    private void checkHardware() {
        status("********* checkHardware  ****");

        if (!checkPermissions(activity)) return;

        LocationManager locationManager = (LocationManager) activity.getSystemService(Context.LOCATION_SERVICE);
        if (!locationManager.isProviderEnabled(LocationManager.GPS_PROVIDER)) {
            Intent intent = new Intent(Settings.ACTION_LOCATION_SOURCE_SETTINGS);
            activity.startActivityForResult(intent, GPS_REQUEST_CODE);
        }

        mBtAdapter = BluetoothAdapter.getDefaultAdapter();
        if (mBtAdapter == null) {
            Log.e(TAG,"Your device does not support Bluetooth connection");
            return;
        }

        if (!mBtAdapter.isEnabled()) {
            Intent enableBtIntent = new Intent(BluetoothAdapter.ACTION_REQUEST_ENABLE);
            activity.startActivityForResult(enableBtIntent, BT_REQUEST_CODE);
        }
    }
}
