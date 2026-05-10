package com.hoho.android.usbserial.driver

@RequiresApi(Build.VERSION_CODES.N)
public class NmeaActivity : AppCompatActivity(), LocationListener, OnNmeaMessageListener
{
    var PERMISSIONS_REQUEST_ACCESS_FINE_LOCATION_ID = 0x10;
    lateinit var locationManager : LocationManager;
    var datanmea = "";

    override fun onCreate(savedInstanceState: Bundle?) {
        this.initGPSgettingLogic()
    }

    override fun onRequestPermissionsResult(requestCode: Int, permissions: Array<out String>, grantResults: IntArray) {
        super.onRequestPermissionsResult(requestCode, permissions, grantResults)
        when (requestCode) {
            PERMISSIONS_REQUEST_ACCESS_FINE_LOCATION_ID -> {
                // If request is cancelled, the result arrays are empty.
                if (grantResults.isNotEmpty()
                    && grantResults[0] == PackageManager.PERMISSION_GRANTED
                ) { // granted
                    this.initGPSgettingLogic()
                } else { // denied,
                }
                return
            }
        }
    }

    private fun initGPSgettingLogic() {
        this.locationManager =
            this.getSystemService(Context.LOCATION_SERVICE) as LocationManager
        var gpsEnabled: Boolean = locationManager.isProviderEnabled(
            LocationManager.GPS_PROVIDER
        )
        if (gpsEnabled) { //GPS ON
            Log.d("NMEA_APP", javaClass.name + ":" + "GPS ON :)")
            if (ContextCompat.checkSelfPermission(this,Manifest.permission.ACCESS_FINE_LOCATION) != PackageManager.PERMISSION_GRANTED) { // Permission is not granted
                Log.d("NMEA_APP", javaClass.name + ":" + "Request Permission ")
                // No explanation needed; request the permission
                ActivityCompat.requestPermissions(this, arrayOf(Manifest.permission.ACCESS_FINE_LOCATION),PERMISSIONS_REQUEST_ACCESS_FINE_LOCATION_ID)
            }
            locationManager.requestLocationUpdates(LocationManager.GPS_PROVIDER, 10000, 10000f, this)
            locationManager.addNmeaListener(this)
        } else { //GPS NOT ON
            Log.d("NMEA_APP", javaClass.name + ":" + "GPS NOT ON")
        }

    }

    override fun onLocationChanged(location: Location?) {

    }
    override fun onStatusChanged(provider: String?, status: Int, extras: Bundle?) {

    }

    override fun onProviderEnabled(provider: String?) {

    }

    override fun onProviderDisabled(provider: String?) {

    }

    override fun onNmeaMessage(nmeamessage: String?, timestamp: Long) {
        Log.d(
            "NMEA_APP",
            javaClass.name + ":" + "[" + timestamp + "]" + nmeamessage+ ""
        )
        sendData(nmeamessage)
    }

    private fun sendData(nmea: String?) {
        var data = nmea?.substringBefore(",")
        var title = data?.substringAfter("$")
        var gen = title?.substring(2,5)
        if(gen == "GGA"){
            datanmea = ""
            datanmea += "$nmea\r\n"
        }
        if(gen == "GSA" || gen == "RMC"){
            datanmea += "$nmea\r\n"
        }
        if(gen == "RMC"){
            TextView.append(datanmea)
        }
    }
}
