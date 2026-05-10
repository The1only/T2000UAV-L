// // Java class
// // Debugging... /Users/terjenilsen/AndroidSDK/platform-tools/adb -s 10.19.0.101:5555 logcat | grep TERJE

package com.hoho.android.usbserial.driver;

import android.app.PendingIntent;
import android.app.Activity;
import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;
import android.content.ContextWrapper;
import android.content.IntentFilter;
import android.hardware.usb.UsbDevice;
import android.hardware.usb.UsbDeviceConnection;
import android.hardware.usb.UsbManager;
import android.os.Build;
import android.os.Bundle;
import android.os.Handler;
import android.os.Looper;
import android.text.Spannable;
import android.text.SpannableStringBuilder;
import android.text.method.ScrollingMovementMethod;
import android.text.style.ForegroundColorSpan;
import android.net.Uri;
import android.provider.Settings;

import android.view.LayoutInflater;
import android.view.Menu;
import android.view.MenuInflater;
import android.view.MenuItem;
import android.view.View;
import android.view.ViewGroup;
import android.view.WindowManager;

import android.widget.TextView;
import android.widget.Toast;
import android.widget.ToggleButton;
import android.util.Log;

import java.util.ArrayList;
import java.util.Locale;

import androidx.annotation.NonNull;
import androidx.fragment.app.ListFragment;
import androidx.annotation.NonNull;
import androidx.annotation.Nullable;
import androidx.core.content.ContextCompat;
import androidx.fragment.app.Fragment;

import com.hoho.android.usbserial.util.SerialInputOutputManager;
import com.hoho.android.usbserial.driver.UsbSerialDriver;
import com.hoho.android.usbserial.driver.CustomProber;
import com.hoho.android.usbserial.util.HexDump;

import java.net.InetAddress;
import java.net.DatagramPacket;
import java.net.DatagramSocket;

import java.io.IOException;
import java.util.Arrays;
import java.util.EnumSet;
import java.util.Objects;
import java.util.ArrayDeque;
import java.util.Queue;

public class TestClassTerje implements SerialInputOutputManager.Listener {

    private String TAG = "USBLOG";
    public long nativeHandle = 0;   // instance field, not static

    // Device id range you care about
    private final int deviceId1 = 1000;
    private final int deviceId2 = 1100;

    private enum UsbPermission { Unknown, Requested, Granted, Denied }
    private final String INTENT_ACTION_GRANT_USB = BuildConfig.APPLICATION_ID + ".GRANT_USB";
    private final int WRITE_WAIT_MILLIS = 200;

    private boolean connected = false;
    private SerialInputOutputManager usbIoManager;
    private int baudRate=9600; // Default for the scanner...
    private int g_portNum = 0;
    private UsbSerialPort usbSerialPort;
    private UsbPermission usbPermission = UsbPermission.Unknown;
    private final boolean withIoManager = true; // event-driven ON
    private final Handler mainLooper;
    private final Context mcontext;
    private static int[] used = new int[5];
    private static String[] _serialnum = { "", "", "", "", "" };
    private static boolean _first = true;

    private String serialNum = "";

    // Optional pull queue (raw chunks as received)
    private final Object qLock = new Object();
    private final Queue<byte[]> rxQueue = new ArrayDeque<>(64);

    // ===== JNI callbacks (implement these in C++) =====
    static {
        try { System.loadLibrary("serialbridge"); } catch (Throwable ignore) {}
    }

    private native void nativeOnSerialBytes(byte[] data);
    private native void nativeOnConnected(boolean connected);
    private native void nativeOnSerialError(String message);

    private static class BuildConfig {
        public static final boolean DEBUG = Boolean.parseBoolean("true");
        public static final String APPLICATION_ID = "com.hoho.android.usbserial.examples";
        public static final String BUILD_TYPE = "debug";
    }

    private void status(String s) { Log.d(TAG, s); }

    public TestClassTerje(Context contextIn) {
        this.mcontext = contextIn;
        this.mainLooper = new Handler(Looper.getMainLooper());
    }

    // ---------------------- Public API (called from C++) ----------------------

    /** Returns last known device serial (string meta info; not used for I/O). */
    public String getInfo() { return this.serialNum; }

    /** Disconnect and return a tiny status (purely informational). */
    public String disconn() { disconnect(); return "Discon"; }

    /** Send raw bytes exactly as provided. */
    public int sendToSerial(byte[] bytes) { return send(bytes); }

    /** Pull next received chunk (or null if none). */
    public byte[] recFromSerial() {
        synchronized (qLock) { return rxQueue.poll(); }
    }

    /** Connect by numeric port index (0..n). */
    public String getconnected(int port, int baudrate) {
        this.TAG = "USBLOG_" + port;
        this.baudRate = baudrate;
        connect(port);
        return String.valueOf(this.connected);
    }

    /** Connect by matching device serial string + set baudrate. */
    public String connectserial(String serial, int baudrate) {
        this.TAG = "USBLOG_" + serial;
        this.baudRate = baudrate;
        connects(serial);
        return String.valueOf(connected);
    }

    // ---------------------- SerialInputOutputManager.Listener -----------------
    public void setNativeHandle(long ptr) {
        nativeHandle = ptr;

        if(_first == true){
            buildSerialNumbers();
            _first = false;
        }

        android.util.Log.d("USBLOG", "setNativeHandle=" + ptr);
    }

    /** Event-driven receive: push raw bytes to C++ and queue for optional pull. */
    @Override
    public void onNewData(byte[] data) {
        try { nativeOnSerialBytes(data); } catch (Throwable ignore) {}
    }

    @Override
    public void onRunError(Exception e) {
        mainLooper.post(() -> {
            status("IO loop ended: " + e.getMessage());
            try { nativeOnSerialError(e.getMessage()); } catch (Throwable ignore) {}
            disconnect();
        });
    }

    // ---------------------- Connection management -----------------------------

    private static String norm(String s) {
        if (s == null) return null;
        s = s.trim();
        return s.replace("\u200B", "").replace("\uFEFF", "");
    }

    private void buildSerialNumbers(){
        status("TERJE::: build serial");
        for (int i = 0; i <= 5; i++) {
            connect(i);
            if (connected) {
                _serialnum[i] = serialNum;
                status(String.format(Locale.US, "Device serial: '%s'", _serialnum[i]));
                disconnect();
            }
        }
    }

    private void connects(String targetSerial) {
        for (int i = 0; i <= 5; i++) {
            String a = norm(targetSerial);
            String b = norm(_serialnum[i]);
            status(String.format(Locale.US, "Compare serial: '%s' vs '%s'", a, b));
            if (Objects.equals(a, b)) {
                connect(i);
                if (connected) {
                    return;
                }
            }
        }
    }

// ---------------------- Connection management -----------------------------

    private void connect(int portNum) {
        UsbDevice device = null;
        UsbManager usbManager = (UsbManager) mcontext.getSystemService(Context.USB_SERVICE);

        for (UsbDevice v : usbManager.getDeviceList().values()) {
            if (v.getDeviceId() > deviceId1 && v.getDeviceId() < deviceId2) {
                if (portNum == 0) {
                    device = v;
                    break;
                }
                portNum--;
            }
        }
        if (device == null) { status("connection failed: device not found"); return; }

        UsbSerialDriver driver = UsbSerialProber.getDefaultProber().probeDevice(device);
        if (driver == null) driver = CustomProber.getCustomProber().probeDevice(device);
        if (driver == null) { status("connection failed: no driver for device"); return; }

        usbSerialPort = driver.getPorts().get(0);
        UsbDeviceConnection usbConnection = usbManager.openDevice(driver.getDevice());

        if (usbConnection == null && usbPermission == UsbPermission.Unknown && !usbManager.hasPermission(driver.getDevice())) {
            usbPermission = UsbPermission.Requested;
            int flags = Build.VERSION.SDK_INT >= Build.VERSION_CODES.M ? PendingIntent.FLAG_MUTABLE : 0;
            Intent intent = new Intent(INTENT_ACTION_GRANT_USB);
            intent.setPackage(mcontext.getPackageName());
            PendingIntent usbPermissionIntent = PendingIntent.getBroadcast(mcontext, 0, intent, flags);
            usbManager.requestPermission(driver.getDevice(), usbPermissionIntent);
            return;
        }
        if (usbConnection == null) {
            status(!usbManager.hasPermission(driver.getDevice()) ? "permission denied" : "open failed");
            return;
        }

        try {
            usbSerialPort.open(usbConnection);
            getInfo(driver);

            try {
                usbSerialPort.setParameters(baudRate, 8, UsbSerialPort.STOPBITS_1, UsbSerialPort.PARITY_NONE);
            } catch (UnsupportedOperationException e) {
                status("setParameters unsupported: " + e.getMessage());
            }

            if (withIoManager) {
                usbIoManager = new SerialInputOutputManager(usbSerialPort, this);
                usbIoManager.start();
            }

            connected = true;
            g_portNum = portNum;
            usbSerialPort.setDTR(true);
            status("connected");
            try { nativeOnConnected(true); } catch (Throwable ignore) {}

        } catch (Exception e) {
            status("connection failed: " + e.getMessage());
            disconnect();
        }
    }

    private void getInfo(UsbSerialDriver driver) {
        UsbDevice usbDev = driver.getDevice();
        try {
            serialNum = usbDev.getSerialNumber();
            status("Serial=" + serialNum);
        } catch (Exception e) {
            serialNum = "";
            status("Serial not available: " + e.getMessage());
        }
    }

    private void disconnect() {
        if (!connected && usbIoManager == null && usbSerialPort == null) return;

        connected = false;

        if (usbIoManager != null) {
            usbIoManager.setListener(null);
            usbIoManager.stop();
            usbIoManager = null;
        }
        if (usbSerialPort != null) {
            try { usbSerialPort.close(); } catch (IOException ignore) {}
            usbSerialPort = null;
        }
        usbPermission = UsbPermission.Unknown;

        try { nativeOnConnected(false); } catch (Throwable ignore) {}
        try { Thread.sleep(100); } catch (InterruptedException ignore) {}
    }

    // ---------------------- Raw send (bytes) ---------------------------------

    private int send(byte[] data) {
        if (!connected) {
            status("not connected");
            try {
                disconnect();
                Thread.sleep(100);
                connect(g_portNum);
                Thread.sleep(100);
            } catch (Exception e) { status("send reconnect failed: " + e.getMessage()); }
            return -1;
        }
        try {
            usbSerialPort.write(data, WRITE_WAIT_MILLIS);
            return data.length;
        } catch (Exception e) {
            onRunError(e);
            return -1;
        }
    }

    // ---------------------- Control lines ------------------------------------

    public int ControlLines(boolean rts /*cts ignored; it's an input*/) {
        if (!connected) return -1;
        try {
            usbSerialPort.setRTS(rts);
            usbSerialPort.setDTR(true);
            EnumSet<UsbSerialPort.ControlLine> lines = usbSerialPort.getControlLines();
            // Example: boolean cts = lines.contains(UsbSerialPort.ControlLine.CTS);
        } catch (Exception e) {
            status("control lines failed: " + e.getMessage());
        }
        return (rts ? 1 : 0);
    }

// ---------------------- Control lines ------------------------------------

    public int change(int n)
    {
        int r = setScreenBrightness( mcontext, n);
        return r;
    }

    //  finally use below method for set brightness
    private int setScreenBrightness(Context xContext,int brightnessValue){
        // Make sure brightness value between 0 to 255

        if (!Settings.System.canWrite(xContext))
        {
            showBrightnessPermissionDialog(xContext);
            return -1;
        }

        if(brightnessValue >= 0 && brightnessValue <= 255){
            Settings.System.putInt(
                xContext.getContentResolver(),
                Settings.System.SCREEN_BRIGHTNESS,
                brightnessValue
            );
        }
        return brightnessValue;
    }

    private static void showBrightnessPermissionDialog(final Context context) {

        Intent intent = new Intent(android.provider.Settings.ACTION_MANAGE_WRITE_SETTINGS);
        intent.setData(Uri.parse("package:" + context.getPackageName()));
        intent.addFlags(Intent.FLAG_ACTIVITY_NEW_TASK);
        context.startActivity(intent);
    }

}
