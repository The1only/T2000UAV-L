package com.wit.witsdk.Device;

import androidx.appcompat.app.AppCompatActivity;

import android.app.AlertDialog;
import android.content.Intent;
import android.os.Bundle;
import android.text.InputType;
import android.util.Log;
import android.view.View;
import android.widget.ArrayAdapter;
import android.widget.Button;
import android.widget.EditText;
import android.widget.Spinner;
import android.widget.TextView;
import android.widget.Toast;
import android.content.Context;

import com.wit.witsdk.Device.DeviceManager;
import com.wit.witsdk.Device.DeviceModel;
import com.wit.witsdk.Device.Interface.DeviceDataListener;

import java.util.ArrayList;
import java.util.List;
import java.util.Timer;
import java.util.TimerTask;

public class DeviceActivity extends AppCompatActivity implements DeviceDataListener {

    private static final String TAG = "WitLOG";
    private Context mcontext;
    private DeviceModel deviceModel;
    private DeviceManager deviceManager = DeviceManager.getInstance();
    private TextView dataView;
    private Timer timer;

    public DeviceActivity(Context contextIn)
    {
        mcontext = contextIn;
    }

    @Override
    protected void onResume() {
        super.onResume();
        if(timer!=null){
            timer.cancel();
            timer = null;
        }
        startUpdate();
    }

    @Override
    protected void onDestroy() {
        super.onDestroy();
        if(timer!=null){
            timer.cancel();
            timer = null;
        }
        deviceManager.RemoveDeviceListener(this);
    }

    /**
     * Enable data refresh
     * */
    private void startUpdate(){
        if(deviceModel == null){
            return;
        }

        timer = new Timer();
        timer.scheduleAtFixedRate(new TimerTask() {
            @Override
            public void run() {
                runOnUiThread(new Runnable() {
                    @Override
                    public void run() {
                        dataView.setText(deviceModel.GetDataDisplay());
                    }
                });
            }
        }, 1000, 100);
    }

    /**
     * Sending sensor data
     * */
    private void SendDeviceDataByMsg(byte reg, String msg, ArrayList<String> list){
        if(deviceModel == null){
            return;
        }

        Spinner spinner = new Spinner(this);
        ArrayAdapter<String> adapter = new ArrayAdapter<>(this, android.R.layout.simple_spinner_item, list);
        adapter.setDropDownViewResource(android.R.layout.simple_spinner_dropdown_item);
        spinner.setAdapter(adapter);

        // Create AlertDialog and set the title
        AlertDialog dialog = new AlertDialog.Builder(this)
                .setTitle(msg)
                .setView(spinner)
                .setPositiveButton("OK", null)
                .setNegativeButton("Cancel", (dialogInterface, which) -> dialogInterface.cancel())
                .create();

        dialog.show();

        dialog.getButton(AlertDialog.BUTTON_POSITIVE).setOnClickListener(v -> {
            // Retrieve the text input by the user
            String userInput = (String) spinner.getSelectedItem();
            if (!userInput.isEmpty()) {
                try {
                    int value = Integer.parseInt(userInput);
                    if(reg == 0x03){
                        deviceModel.SetRRate(value);
                    }
                    else if(reg == 0x1f){
                        deviceModel.SetBandwidth(value);
                    }
                }
                catch (Exception ex){
                    Log.i(TAG, "Error", ex);
                }
            }
            // close Dialog
            dialog.dismiss();
        });
    }

    /**
     * Magnetic field calibration
     * */
    public void MagCalibration(boolean btn){
        if(deviceModel == null){
            return;
        }
        if(btn){
            deviceModel.SetMagStart();
        }
        else {
            deviceModel.SetMagStop();
        }
    }

    /**
     * Real time data callback of sensors
     * */
    @Override
    public void OnReceive(String deviceName, String displayData) {

    }

    /**
     * When the sensor connection status changes
     * */
    @Override
    public void OnStatusChange(String deviceName, boolean status) {
        if(status){
            Log.i(TAG, deviceName + "  Connected");
        }
        else {
            Log.i(TAG, deviceName + "   Disconnect");
        }
    }
}
