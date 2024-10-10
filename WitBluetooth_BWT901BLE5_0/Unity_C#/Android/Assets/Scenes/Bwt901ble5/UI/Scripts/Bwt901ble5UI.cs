using System;
using System.Collections;
using System.Collections.Generic;
using System.Threading;
using UnityEngine;
using UnityEngine.UI;
using Wit.SDK.Modular.Sensor.Modular.DataProcessor.Constant;

/// <summary>
/// ����5.0ʾ������
/// Bluetooth 5.0 Example Program
/// </summary>
public class Bwt901ble5UI : MonoBehaviour
{
    /// <summary>
    /// ����������
    /// Bluetooth manager
    /// </summary>
    BluetoothManager bluetoothManager;

    /// <summary>
    /// �ҵ����豸
    /// Found device
    /// </summary>
    private Dictionary<string, BWT901BLE> FoundDeviceDict = new Dictionary<string, BWT901BLE>();

    /// <summary>
    /// ����3d Box�¼�
    /// Update Cube events
    /// </summary>
    /// <param name="bWT901BLE"></param>
    public delegate void UpdateBoxHandler(BWT901BLE bWT901BLE);

    public static event UpdateBoxHandler UpdateBox;

    #region ������ Binding Properties
    public Button ScanBtn;

    public Button StopBtn;

    public Button ConnectBtn;

    public Text ConnectText;

    public Text MsgText;

    public Text DeviceName;

    public Button RRateBtn_1;

    public Button RRateBtn_2;

    public Button AccBtn;

    public Text Info;

    public Text AccX;
    public Text AccY;
    public Text AccZ;
    public Text AsX;
    public Text AsY;
    public Text AsZ;
    public Text AngX;
    public Text AngY;
    public Text AngZ;
    #endregion

    string currentMac = null;

    // Start is called before the first frame update
    void Start()
    {
        MsgText.text = "ά����������5.0ʾ������";
        InitBlueManager();
        ScanBtn.onClick.AddListener(StartScan);
        StopBtn.onClick.AddListener(StopScan);
        ConnectBtn.onClick.AddListener(Connect);
        RRateBtn_1.onClick.AddListener(SetRRate10);
        RRateBtn_2.onClick.AddListener(SetRRate200);
        AccBtn.onClick.AddListener(AccCeil);
    }

    /// <summary>
    /// ��ʼ������������
    /// Initialize Bluetooth Manager
    /// </summary>
    private void InitBlueManager()
    {
        // ������������� Get Bluetooth Manager
        bluetoothManager = BluetoothManager.Instance;
        // �������豸�¼� Bind search device events
        bluetoothManager.OnDeviceFound -= WitBluetoothManager_OnDeviceFound;
        bluetoothManager.OnDeviceFound += WitBluetoothManager_OnDeviceFound;
    }

    /// <summary>
    /// �ҵ��豸ʱ��ص��������
    /// This method will be called back when the device is found
    /// </summary>
    /// <param name="mac"></param>
    /// <param name="deviceName"></param>
    private void WitBluetoothManager_OnDeviceFound(string mac, string deviceName)
    {
        // ���ƹ���
        // Name filtering
        if (deviceName != null && deviceName.Contains("WT"))
        {
            if (!FoundDeviceDict.ContainsKey(mac))
            {
                BWT901BLE bWT901BLE = new BWT901BLE();
                bWT901BLE.SetDeviceAddress(mac);
                bWT901BLE.SetDeviceName(deviceName);
                FoundDeviceDict.Add(mac, bWT901BLE);
                MsgText.text = $"�ҵ��豸{deviceName}({mac})";
                DeviceName.text = $"{deviceName}({mac})";
                currentMac = mac;
            }
        }
    }

    /// <summary>
    /// �����豸
    /// Connected Device
    /// </summary>
    private void Connect()
    {
        if (FoundDeviceDict.Count == 0) {
            MsgText.text = "���������豸��";
            return;
        }
        BWT901BLE bWT901BLE = FoundDeviceDict[currentMac];
        if (ConnectText.text.Contains("Connect")) {
            // ������豸
            // Open this device
            MsgText.text = "���������豸";
            bWT901BLE.Open();
            bWT901BLE.OnUpdate += BWT901BLE_OnUpdate;
            Info.text = bWT901BLE.GetDeviceName();
            MsgText.text = "�����豸�ɹ�";
            ConnectText.text = "Disconnect";
            UpdateBox?.Invoke(bWT901BLE);
        }
        else if (ConnectText.text.Contains("Disconnect")) {
            bWT901BLE.OnUpdate -= BWT901BLE_OnUpdate;
            bWT901BLE.Close();
            MsgText.text = "�ر��豸�ɹ�";
            ConnectText.text = "Connect";
        }
    }

    /// <summary>
    /// ���������ݸ���ʱ��ص�������� This method will be called back when sensor data is updated
    /// </summary>
    /// <param name="BWT901BLE"></param>
    private void BWT901BLE_OnUpdate(BWT901BLE BWT901BLE)
    {
        DisplayData(BWT901BLE);
    }

    /// <summary>
    /// ����չʾ
    /// Data display
    /// </summary>
    /// <param name="bWT901BLE"></param>
    private void DisplayData(BWT901BLE bWT901BLE)
    {
        AccX.text = "AccX:" + bWT901BLE.GetDeviceData(WitSensorKey.AccX);
        AccY.text = "AccY:" + bWT901BLE.GetDeviceData(WitSensorKey.AccY);
        AccZ.text = "AccZ:" + bWT901BLE.GetDeviceData(WitSensorKey.AccZ);
        AsX.text = "AsX:" + bWT901BLE.GetDeviceData(WitSensorKey.AsX);
        AsY.text = "AsY:" + bWT901BLE.GetDeviceData(WitSensorKey.AsY);
        AsZ.text = "AsZ:" + bWT901BLE.GetDeviceData(WitSensorKey.AsZ);
        AngX.text = "AngX:" + bWT901BLE.GetDeviceData(WitSensorKey.AngleX);
        AngY.text = "AngY:" + bWT901BLE.GetDeviceData(WitSensorKey.AngleY);
        AngZ.text = "AngZ:" + bWT901BLE.GetDeviceData(WitSensorKey.AngleZ);
    }

    /// <summary>
    /// ֹͣɨ��
    /// Stop Scan
    /// </summary>
    private void StopScan()
    {
        MsgText.text = "ֹͣɨ��";
        bluetoothManager.stopScan();
    }

    /// <summary>
    /// ��ʼɨ��
    /// Start Scan
    /// </summary>
    private void StartScan()
    {
        MsgText.text = "��ʼɨ��";
        FoundDeviceDict.Clear();
        bluetoothManager.startScan();
    }

    /// <summary>
    /// ���ٶ�У׼
    /// Acceleration calibration
    /// </summary>
    private void AccCeil()
    {
        MsgText.text = "���ڽ��мӼ�У׼����ȴ�5��";
        BWT901BLE bWT901BLE = FoundDeviceDict[currentMac];
        bWT901BLE.AppliedCalibration();
        MsgText.text = "�Ӽ�У׼���";
    }

    /// <summary>
    /// 200hz�ش�
    /// 200Hz return
    /// </summary>
    private void SetRRate200()
    {
        BWT901BLE bWT901BLE = FoundDeviceDict[currentMac];
        bWT901BLE.SetReturnRate(0x0b);
        MsgText.text = "����200hz���";
    }

    /// <summary>
    /// 10hz�ش�
    /// 10Hz return
    /// </summary>
    private void SetRRate10()
    {
        BWT901BLE bWT901BLE = FoundDeviceDict[currentMac];
        bWT901BLE.SetReturnRate(0x06);
        MsgText.text = "����10hz���";
    }

    // Update is called once per frame
    void Update()
    {
        
    }
}
