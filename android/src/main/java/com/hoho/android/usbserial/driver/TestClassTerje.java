// Java class
package com.hoho.android.usbserial.driver;

import android.app.PendingIntent;
import android.app.Activity;
import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;
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
import android.view.LayoutInflater;
import android.view.Menu;
import android.view.MenuInflater;
import android.view.MenuItem;
import android.view.View;
import android.view.ViewGroup;
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
//import com.hoho.android.usbserial.examples;

import java.net.InetAddress;
import java.net.DatagramPacket;
import java.net.DatagramSocket;

import java.io.IOException;
import java.util.Arrays;
import java.util.EnumSet;

//public class TestClassTerje extends ListFragment implements SerialInputOutputManager.Listener
//public class TestClassTerje extends Activity implements SerialInputOutputManager.Listener
public class TestClassTerje implements SerialInputOutputManager.Listener
{
    private static final String TAG = "USBLOG";

    private enum UsbPermission { Unknown, Requested, Granted, Denied }
    private static final String INTENT_ACTION_GRANT_USB = BuildConfig.APPLICATION_ID + ".GRANT_USB";
    private static final int WRITE_WAIT_MILLIS = 200;
    private static final int READ_WAIT_MILLIS = 200;
    private static byte[] received = new byte[0];

    private boolean connected = false;
    private SerialInputOutputManager usbIoManager;
    private int deviceId1, deviceId2, portNum, baudRate;
    private UsbSerialPort usbSerialPort;
    private UsbPermission usbPermission = UsbPermission.Unknown;
    private boolean withIoManager = false;
    private TextView receiveText;

    private BroadcastReceiver broadcastReceiver;
    private Handler mainLooper;
    private Context mcontext;
//    private static Context mcontext;

    private static class BuildConfig {
      public static final boolean DEBUG = Boolean.parseBoolean("true");
      public static final String APPLICATION_ID = "com.hoho.android.usbserial.examples";
      public static final String BUILD_TYPE = "debug";
    }


    void status(String str) {
    //       System.out.println(str);
 //        Log.i(TAG, str);
    }


    public TestClassTerje(Context contextIn)
    {
        mcontext = contextIn;
        deviceId1 = 1002;
        deviceId2 = 1005;
        portNum = 0;
        baudRate = 9600; //dx115200;
        withIoManager = false;

        status("********* TestClassTerje **********");
        Intent intent = new Intent(UsbManager.EXTRA_PERMISSION_GRANTED);
/*
        broadcastReceiver = new BroadcastReceiver() {
        @Override
        public void onReceive(Context context, Intent intent) {
            */
            status("############# TERJE : Received..." + INTENT_ACTION_GRANT_USB.equals(intent.getAction()));
            if(INTENT_ACTION_GRANT_USB.equals(intent.getAction())) {
                usbPermission = intent.getBooleanExtra(UsbManager.EXTRA_PERMISSION_GRANTED, false) ? UsbPermission.Granted : UsbPermission.Denied;
                status("############# TERJE x: " + usbPermission);
             //   connect();
     //           }
            }
   //     };

        mainLooper = new Handler(Looper.getMainLooper());

        status("############# TERJE: Constructor completed...");
        connect();
    }

    public String disconn() {
        disconnect();
        return "Disconnected...";
    }

    public int sendToSerial(String str) {
        int x = send(str);
        return x;
    }

    public String recFromSerial() {
        String x = read();
        return x;
    }

    public String getconnected() {
        String x = String.valueOf(connected);
        return x;
    }

//---------------------------------------------------------------------------------
//---------------------------------------------------------------------------------

    public byte[] concatenateByteArrays(byte[] a, byte[] b) {
        byte[] result = new byte[a.length + b.length];
        System.arraycopy(a, 0, result, 0, a.length);
        System.arraycopy(b, 0, result, a.length, b.length);
        return result;
    }

    @Override
    public void onRunError(Exception e) {
        mainLooper.post(() -> {
                  status("connection lost: " + e.getMessage());
                  disconnect();
              });
    }

    /*
     * Serial
     */
    public String onNewDatax(byte[] data) {
        if(received.length < 2048)
            received = concatenateByteArrays(received,data);

        if(data[data.length-1] < 0x20) {
            String lText = new String(received, 0, received.length);
//            status("received: " + lText);
            received = new byte[0];
            return lText;
        }
        return "";
    }

    @Override
    public void onNewData(byte[] data) {
        if(received.length < 2048)
            received = concatenateByteArrays(received,data);

        if(data[data.length-1] < 0x20) {
            String lText = new String(received, 0, received.length);
            status("received: " + lText);
            received = new byte[0];
        }
    }
/*
     * Serial + UI
*/
    private void connect() {
        UsbDevice device = null;

        status("############# TERJE : Now we are connecting...");

        // Get UsbManager from Android
        UsbManager usbManager = (UsbManager) mcontext.getSystemService(mcontext.USB_SERVICE);

        for(UsbDevice v : usbManager.getDeviceList().values()){
            status(v.getDeviceId() + " Found...");
            if(v.getDeviceId() > 1000 && v.getDeviceId() < 1100)   //== deviceId1 || v.getDeviceId() == deviceId2)
                device = v;
        }
        if(device == null) {
            status("connection failed: device not found");
            return;
        }
        UsbSerialDriver driver = UsbSerialProber.getDefaultProber().probeDevice(device);
        if(driver == null) {
            driver = CustomProber.getCustomProber().probeDevice(device);
        }
        if(driver == null) {
            status("connection failed: no driver for device");
            return;
        }
        status("Portnum: " + portNum);
        if(driver.getPorts().size() < portNum) {
            status("connection failed: not enough ports at device");
            return;
        }
        usbSerialPort = driver.getPorts().get(portNum);
        UsbDeviceConnection usbConnection = usbManager.openDevice(driver.getDevice());
        if(usbConnection == null && usbPermission == UsbPermission.Unknown && !usbManager.hasPermission(driver.getDevice())) {
            usbPermission = UsbPermission.Requested;
            int flags = Build.VERSION.SDK_INT >= Build.VERSION_CODES.M ? PendingIntent.FLAG_MUTABLE : 0;
            Intent intent = new Intent(INTENT_ACTION_GRANT_USB);
            intent.setPackage(mcontext.getPackageName());
            PendingIntent usbPermissionIntent = PendingIntent.getBroadcast(mcontext, 0, intent, flags);
            usbManager.requestPermission(driver.getDevice(), usbPermissionIntent);
            return;
        }
        if(usbConnection == null) {
            if (!usbManager.hasPermission(driver.getDevice()))
                status("connection failed: permission denied");
            else
                status("connection failed: open failed");
            return;
        }

        try {
            usbSerialPort.open(usbConnection);
            try{
                usbSerialPort.setParameters(baudRate, 8, 1, UsbSerialPort.PARITY_NONE);
            }catch (UnsupportedOperationException e){
                status("unsupport setparameters");
            }
            if(withIoManager) {
                usbIoManager = new SerialInputOutputManager(usbSerialPort, this);
                usbIoManager.start();
            }

            status("connected");
            connected = true;
         //   ControlLines();

        } catch (Exception e) {
            status("connection failed: " + e.getMessage());
            disconnect();
        }
    }

    private void disconnect() {
        status("############# TERJE: disconnect ...");

        connected = false;
    //    controlLines.stop();
        if(usbIoManager != null) {
            usbIoManager.setListener(null);
            usbIoManager.stop();
            usbIoManager = null;
        }
        if(usbSerialPort != null){
            try {
                usbSerialPort.close();
            } catch (IOException ignored) {}
            usbSerialPort = null;
        }
        usbPermission = UsbPermission.Unknown;
        try {
            Thread.sleep(100);
        } catch (Exception e) {
            status("disconnect failed: " + e.getMessage());
        }
    }

    private int send(String str) {
        int stat = 0;
        if(!connected) {
            status("not connected");
            try {
                disconnect();
                Thread.sleep(100);
                connect();
                Thread.sleep(100);
            } catch (Exception e) {
                status("send reconnect failed: " + e.getMessage());
            }

//            Toast.makeText(mcontext, "not connected", Toast.LENGTH_SHORT).show();
            return -1;
        }
        try {
            byte[] data = (str + '\n'+'\r').getBytes();
            usbSerialPort.write(data, WRITE_WAIT_MILLIS);
            stat = data.length;
        } catch (Exception e) {
            onRunError(e);
        }
        return stat;
    }

    private String read() {
        if(!connected) {
            status("not connected read");
            try {
                disconnect();
                Thread.sleep(100);
                connect();
                Thread.sleep(100);
            } catch (Exception e) {
                status("read reconnect failed: " + e.getMessage());
            }
//            Toast.makeText(mcontext, "not connected", Toast.LENGTH_SHORT).show();
            return "";
        }
        try {
            byte[] buffer = new byte[8192];
            int len = usbSerialPort.read(buffer, READ_WAIT_MILLIS);
            if(len > 0) {
                return onNewDatax(Arrays.copyOf(buffer, len));
            }
        } catch (IOException e) {
            // when using read with timeout, USB bulkTransfer returns -1 on timeout _and_ errors
            // like connection loss, so there is typically no exception thrown here on error
            status("connection lost: " + e.getMessage());
            disconnect();
        }
        return "";
    }

    public int ControlLines(boolean rts, boolean cts) {
        if (!connected)
            return -1;
        try {
            usbSerialPort.setRTS(rts);
            usbSerialPort.setRTS(cts);

            EnumSet<UsbSerialPort.ControlLine> controlLinesx = usbSerialPort.getControlLines();
            status("rts " + controlLinesx.contains(UsbSerialPort.ControlLine.RTS)); //(UsbSerialPort.ControlLine.RTS));
            status("cts " + controlLinesx.contains(UsbSerialPort.ControlLine.CTS)); //(UsbSerialPort.ControlLine.CTS));
            status("dtr " + controlLinesx.contains(UsbSerialPort.ControlLine.DTR)); //(UsbSerialPort.ControlLine.DTR));
            status("dsr " + controlLinesx.contains(UsbSerialPort.ControlLine.DSR)); //(UsbSerialPort.ControlLine.DSR));
            status("cd  "  + controlLinesx.contains(UsbSerialPort.ControlLine.CD)); //(UsbSerialPort.ControlLine.CD));
            status("ri  "  + controlLinesx.contains(UsbSerialPort.ControlLine.RI)); //(UsbSerialPort.ControlLine.RI));

            // ...

        } catch (Exception e) {
            status("getSupportedControlLines() failed: " + e.getMessage());
        }
        return ((rts==true)?1:0) + ((cts==true)?2:0);
    }
}

// ---------------------------------------------------
// ---------------------------------------------------
