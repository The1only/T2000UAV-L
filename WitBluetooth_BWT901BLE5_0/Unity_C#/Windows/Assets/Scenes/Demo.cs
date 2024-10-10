using Assets.Bluetooth;
using Assets.Device;
using System;
using System.Collections;
using System.Collections.Generic;
using System.Security.Cryptography;
using System.Text;
using TMPro;
using UnityEngine;
using UnityEngine.EventSystems;
using UnityEngine.UI;

/*
 * ʾ�������Ψһ�����ű��������е������߼���������ʵ��
 * The script file corresponding to the unique scenario of the sample program, where all logic in the scenario is implemented
 */
public class Demo : MonoBehaviour
{
    #region ���ֶ� Binding Fields
    // ɨ�谴ť Scan button
    public Text deviceScanButtonText;

    // ɨ��״̬ Scanning status
    public Text deviceScanStatusText;

    // ��ǰ���õ��豸���� The name of the currently configured device
    public Text configDeviceName;

    // ��������� Input Box Data
    public InputField writeInput;

    // �����б� Search List
    public GameObject deviceScanResult;
    Transform scanResultRoot;

    // �����б� Data List
    public GameObject deviceDataResult;
    Transform dataResultRoot;
    #endregion

    #region �����ֶ� Attribute Field
    private bool isScan = false;

    // �ҵ����豸�б� List of devices found
    private List<DeviceModel> findList = new List<DeviceModel>();

    // ��������б� Data component list
    private Dictionary<string, GameObject> dataDict = new Dictionary<string, GameObject>();

    // �豸������ Device Manager
    private DevicesManager devicesManager;

    // �ϴθ���ʱ�� Last update time
    private DateTime lastUpdate;
    #endregion

    // Start is called before the first frame update
    void Start()
    {
        // ��ʼ��ɨ�����������б� Initialize scan results and data list
        scanResultRoot = deviceScanResult.transform.parent;
        deviceScanResult.transform.SetParent(null);
        dataResultRoot = deviceDataResult.transform.parent;
        deviceDataResult.transform.SetParent(null);
        // ����豸������ Obtain Device Manager
        devicesManager = DevicesManager.Instance;
        // ��ʼ��ˢ��ʱ�� Initialize refresh time
        lastUpdate = DateTime.Now;  
    }

    // Update is called once per frame
    void Update()
    {
        // ������������豸 Add searched devices
        if (isScan) {
            if (findList.Count > 0) {
                DeviceModel model = findList[0];
                GameObject g = Instantiate(deviceScanResult, scanResultRoot);
                g.name = model.deviceId;
                g.transform.GetChild(0).GetComponent<Text>().text = model.deviceName;
                g.transform.GetChild(1).GetComponent<Text>().text = model.deviceId;
                g.transform.GetChild(2).GetComponent<Toggle>().isOn = false;
                findList.RemoveAt(0);   
            }
        }

        // ˢ������ Refresh data
        UpdateData();
    }

    private void UpdateData()
    {
        // UI��Ƶ  UI frequency reduction
        //if ((DateTime.Now - lastUpdate).TotalMilliseconds < 100)
        //{
        //    return;
        //}
        try
        {
            foreach (string key in dataDict.Keys)
            {
                GameObject g = dataDict[key];
                DeviceModel model = devicesManager.GetDevice(key);
                if (g != null && model != null)
                {
                    g.transform.GetChild(1).GetComponent<Text>().text = model.GetDataDisplay();
                }
            }
            lastUpdate = DateTime.Now;  
        }
        catch (Exception)
        {
            Debug.LogError("�������ݳ���");
        }
    }

    /// <summary>
    /// ��ʼ�ͽ���ɨ�� Starting and ending scanning
    /// </summary>
    public void Sacn() {
        BlueScanner scanner = BlueScanner.Instance;
        if (!isScan)
        {
            findList.Clear();
            configDeviceName.text = "No Device";
            dataDict.Clear();
            // ���ɨ���б� Clear scan list
            for (int i = scanResultRoot.childCount - 1; i >= 0; i--) {
                Destroy(scanResultRoot.GetChild(i).gameObject);
            }
            for (int i = dataResultRoot.childCount - 1; i >= 0; i--)
            {
                Destroy(dataResultRoot.GetChild(i).gameObject);
            }

            Debug.Log("��ʼ�����豸");
            scanner.StartScan();
            scanner.OnFindDevice -= OnFindDevice;
            scanner.OnFindDevice += OnFindDevice;
            isScan = true;
            deviceScanButtonText.text = "Stop Scan";
            deviceScanStatusText.text = "Scanning";
        }
        else
        {
            scanner.OnFindDevice -= OnFindDevice;
            Debug.Log("���������豸");
            scanner.StopScan(); 
            isScan = false;
            deviceScanButtonText.text = "Start Scan";
            deviceScanStatusText.text = "Finished";
        }
    }

    /// <summary>
    /// �ҵ��豸���ص��˷��� After finding the device, this method will be called back
    /// </summary>
    /// <param name="deviceId"></param>
    private void OnFindDevice(string deviceName, string deviceId) {
        DeviceModel device = new DeviceModel(deviceName, deviceId);
        devicesManager.AddDevice(device);
        findList.Add(device);
    }

    /// <summary>
    /// �����豸 Connecting devices
    /// </summary>
    public void OpenDevice(GameObject g) {
        
        bool isOpen = g.transform.GetChild(2).GetComponent<Toggle>().isOn;
        string key = g.transform.GetChild(1).GetComponent<Text>().text;
        DeviceModel deviceModel = devicesManager.GetDevice(key);

        if (isOpen) 
        {
            Debug.Log("���ڴ��豸" + deviceModel.deviceName);
            deviceModel?.OpenDevice();
            AddDataObject(deviceModel);

        } else {
            deviceModel?.CloseDevice();
            DestroyDataObject(deviceModel);
        }
    }

    /// <summary>
    /// �������б������һ���豸 Add a data component
    /// </summary>
    private void AddDataObject(DeviceModel deviceModel) {
        if (deviceModel == null) { return; }
        if (!dataDict.ContainsKey(deviceModel.deviceId)) {
            GameObject g = Instantiate(deviceDataResult, dataResultRoot);
            g.name = deviceModel.deviceId + "_data";
            g.transform.GetChild(0).GetComponent<Text>().text = deviceModel.deviceName;
            g.transform.GetChild(1).GetComponent<Text>().text = "���ڼ����豸���ݡ�����";
            dataDict.Add(deviceModel.deviceId, g);
        }
    }

    /// <summary>
    /// �������б����Ƴ�һ���豸 Remove a data component
    /// </summary>
    private void DestroyDataObject(DeviceModel deviceModel) {
        if (deviceModel == null) { return; }
        if (dataDict.ContainsKey(deviceModel.deviceId)) {
            GameObject g = dataDict[deviceModel.deviceId];
            try
            {
                Destroy(g);
            }
            catch (Exception ex)
            {
                Debug.LogError(ex.Message);
            }
            finally
            {
                dataDict.Remove(deviceModel.deviceId);  
            }
        }
    }

    /// <summary>
    /// ѡ���豸�������� Select device for configuration
    /// </summary>
    public void SelectDevice(GameObject obj) {
        string deviceID = obj.name.Replace("_data", "");
        devicesManager.currentKey = deviceID;
        configDeviceName.text = devicesManager.GetCurrentDevice().deviceName;
    }

    /// <summary>
    /// ���ü��ٶ�У׼ Set acceleration calibration
    /// </summary>
    public void BtnAccCalibration_Click() {
        DeviceModel deviceModel = devicesManager.GetCurrentDevice();
        if (deviceModel != null) {
            deviceModel.SendData(new byte[] { 0xff, 0xaa, 0x01, 0x01, 0x00 });
        }
    }

    /// <summary>
    /// ���ýǶȲο� Set angle reference
    /// </summary>
    public void BtnAngleReference_Click() {
        DeviceModel deviceModel = devicesManager.GetCurrentDevice();
        if (deviceModel != null)
        {
            deviceModel.SendData(new byte[] { 0xff, 0xaa, 0x01, 0x08, 0x00 });
            deviceModel.SendData(new byte[] { 0xff, 0xaa, 0x00, 0x00, 0x00 });
        }
    }

    /// <summary>
    /// ���ûش�����10hz Set the return rate to 10Hz
    /// </summary>
    public void BtnRRate10_Click() {
        DeviceModel deviceModel = devicesManager.GetCurrentDevice();
        if (deviceModel != null)
        {
            deviceModel.SendData(new byte[] { 0xff, 0xaa, 0x03, 0x06, 0x00 });
            deviceModel.SendData(new byte[] { 0xff, 0xaa, 0x00, 0x00, 0x00 });
        }
    }

    /// <summary>
    /// ���ûش�����100hz Set the return rate to 100Hz
    /// </summary>
    public void BtnRRate100_Click() {
        DeviceModel deviceModel = devicesManager.GetCurrentDevice();
        if (deviceModel != null)
        {
            deviceModel.SendData(new byte[] { 0xff, 0xaa, 0x03, 0x09, 0x00 });
            deviceModel.SendData(new byte[] { 0xff, 0xaa, 0x00, 0x00, 0x00 });
        }
    }

    /// <summary>
    /// ����ԭʼ����ָ�� Send raw data instruction
    /// </summary>
    public void SendHexData() {
        DeviceModel deviceModel = devicesManager.GetCurrentDevice();
        if (deviceModel != null) {
            string input = writeInput.text.Replace(" ", "").Replace("-", "");
            byte[] payload = HexStringToByteArray(input);
            deviceModel.SendData(payload);  
        }
    }

    /// <summary>
    /// �ַ���תbyte���� String to byte array
    /// </summary>
    private byte[] HexStringToByteArray(string s)
    {
        int NumberChars = s.Length;
        byte[] bytes = new byte[NumberChars / 2];
        for (int i = 0; i < NumberChars; i += 2)
            bytes[i / 2] = Convert.ToByte(s.Substring(i, 2), 16);
        return bytes;
    }
}
