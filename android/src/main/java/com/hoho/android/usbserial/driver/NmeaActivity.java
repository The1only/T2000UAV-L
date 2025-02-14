package com.hoho.android.usbserial.driver;

import android.app.Activity;
import android.Manifest;
import android.content.Context;
import android.content.pm.PackageManager;
import android.location.Location;
import android.location.LocationListener;
import android.location.LocationManager;
import android.location.OnNmeaMessageListener;
import android.os.Build;
import android.os.Bundle;
import android.util.Log;
import android.widget.TextView;
import android.os.Handler;
import android.os.Looper;

import androidx.annotation.RequiresApi;
import androidx.appcompat.app.AppCompatActivity;
import androidx.core.app.ActivityCompat;
import androidx.core.content.ContextCompat;

//public class NmeaActivity extends AppCompatActivity implements LocationListener, OnNmeaMessageListener {
public class NmeaActivity implements LocationListener, OnNmeaMessageListener
{
    private static final String TAG = "NMEALOG";

    private static final int PERMISSIONS_REQUEST_ACCESS_FINE_LOCATION_ID = 0x10;
    private LocationManager locationManager;
    private String datanmea = "";
    private Context mcontext;

    public NmeaActivity(Context contextIn) {
        mcontext = contextIn;
        this.initGPSgettingLogic();
    }

    private void initGPSgettingLogic() {
        Log.d(TAG, "initGPSgettingLogic");
        locationManager = (LocationManager) mcontext.getSystemService(mcontext.LOCATION_SERVICE);
        boolean gpsEnabled = locationManager.isProviderEnabled(LocationManager.GPS_PROVIDER);
        if (gpsEnabled) { // GPS ON
            Log.d(TAG, this.getClass().getName() + ": GPS ON :)");
            if (ContextCompat.checkSelfPermission((Activity) mcontext, Manifest.permission.ACCESS_FINE_LOCATION) != PackageManager.PERMISSION_GRANTED) { // Permission is not granted
                Log.d(TAG, this.getClass().getName() + ": Request Permission ");
                // No explanation needed; request the permission
                ActivityCompat.requestPermissions((Activity) mcontext, new String[]{Manifest.permission.ACCESS_FINE_LOCATION}, PERMISSIONS_REQUEST_ACCESS_FINE_LOCATION_ID);
            }
            Log.d(TAG, "ActivityCompat");

/*
            Thread thread = new Thread() {
               @Override
               public void run() {
                    try {

                        listener = new LocationListener(){
                            @Override
                            public void onLocationChanged(Location location) {
                                Log.d(TAG, "on location changed : "+location.getLatitude() + "---"+ location.getLongitude());
                            //    locationManager.removeUpdates(myLocationListener);
                            }

                            @Override
                            public void onStatusChanged(String s, int i, Bundle bundle) {
                                Log.d(TAG , "Status Changed");
                            }

                            @Override
                            public void onProviderEnabled(String s) {
                                Log.d(TAG , "Provider Enabled");
                            }

                            @Override
                            public void onProviderDisabled(String s) {
                                Log.d(TAG , "Provider Disabled");
                            }
                        };

                        Log.d(TAG, "Requesting location updates");
                        locationManager.requestLocationUpdates(LocationManager.GPS_PROVIDER, 0, 0, listener);

                        while(true) {
                            sleep(1000);
//                            handler.post(this);
                        }
                    } catch (InterruptedException e) {
                        e.printStackTrace();
                    }
                }
            };

            thread.start();

*/

/*
            listener = new LocationListener(){
                @Override
                public void onLocationChanged(Location location) {
                    Log.d(TAG, "on location changed : "+location.getLatitude() + "---"+ location.getLongitude());
                //    locationManager.removeUpdates(myLocationListener);
                }

                @Override
                public void onStatusChanged(String s, int i, Bundle bundle) {
                    Log.d(TAG , "Status Changed");
                }

                @Override
                public void onProviderEnabled(String s) {
                    Log.d(TAG , "Provider Enabled");
                }

                @Override
                public void onProviderDisabled(String s) {
                    Log.d(TAG , "Provider Disabled");
                }
            };

            Log.d(TAG, "Requesting location updates");
            locationManager.requestLocationUpdates(LocationManager.GPS_PROVIDER, 0, 0, listener);
*/

/*
            listener = new LocationListener() {
                @Override
                public void onLocationChanged(Location loc) {
                    double latitude = loc.getLatitude();
                    double longitude = loc.getLongitude();
                    double accuracy = Math.round(loc.getAccuracy());
                    double speed = loc.getSpeed();
                    double dir = loc.getBearing();
                    double alt = loc.getAltitude();

                    Log.d(TAG,"gps"+alt);
                }
            };
*/

          //  locationManager.requestLocationUpdates(LocationManager.GPS_PROVIDER, 10000, 10000f, this);
          //  Log.d(TAG, "requestLocationUpdates");
          //  locationManager.addNmeaListener(this);
            Log.d(TAG, "addNmeaListener");
        } else { // GPS NOT ON
            Log.d(TAG, this.getClass().getName() + ": GPS NOT ON");
        }
    }

    @Override
    public void onLocationChanged(Location location) {
    }

    @Override
    public void onStatusChanged(String provider, int status, Bundle extras) {
    }

    @Override
    public void onProviderEnabled(String provider) {
    }

    @Override
    public void onProviderDisabled(String provider) {
    }

    @Override
    public void onNmeaMessage(String nmeamessage, long timestamp) {
        Log.d(TAG, this.getClass().getName() + ": [" + timestamp + "]" + nmeamessage + "");
   //     sendData(nmeamessage);
    }

    private void sendData(String nmea) {
        String data = nmea != null ? nmea.split(",")[0] : null;
        String title = data != null ? data.split("\\$")[1] : null;
        String gen = title != null ? title.substring(2, 5) : null;
        if ("GGA".equals(gen)) {
            datanmea = "";
            datanmea += nmea + "\r\n";
        }
        if ("GSA".equals(gen) || "RMC".equals(gen)) {
            datanmea += nmea + "\r\n";
        }
        if ("RMC".equals(gen)) {
            Log.i(TAG, datanmea);
        }
    }
}
/*
public class MyLocationListener implements LocationListener {

    @Override
    public void onLocationChanged(Location location) {
        if (ActivityCompat.checkSelfPermission(MapsActivity.this, Manifest.permission.ACCESS_FINE_LOCATION) != PackageManager.PERMISSION_GRANTED && ActivityCompat.checkSelfPermission(MapsActivity.this, Manifest.permission.ACCESS_COARSE_LOCATION) != PackageManager.PERMISSION_GRANTED) {
            return;
        }
        Log.d("maps", "on location changed : "+location.getLatitude() + "---"+ location.getLongitude());
        locationManager.removeUpdates(myLocationListener);
    }

    @Override
    public void onStatusChanged(String s, int i, Bundle bundle) {
        Log.d("maps" , "Status Changed");
    }

    @Override
    public void onProviderEnabled(String s) {
        Log.d("maps" , "Provider Enabled");
    }

    @Override
    public void onProviderDisabled(String s) {
        Log.d("maps" , "Provider Disabled");
    }
}
*/
