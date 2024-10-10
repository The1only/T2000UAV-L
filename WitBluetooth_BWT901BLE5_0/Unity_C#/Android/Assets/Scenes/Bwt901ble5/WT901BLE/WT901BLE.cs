using System.Threading;
using Wit.SDK.Modular.Sensor.Device;
using Wit.SDK.Modular.WitSensorApi.Interface;
using Wit.SDK.Sensor.Connector.Entity;
using Wit.SDK.Sensor.Connector.Role;

public class BWT901BLE : IAttitudeSensorApi
{
    /// <summary>
    /// �豸ģ��
    /// Device Model
    /// </summary>
    private DeviceModel DeviceModel;

    /// <summary>
    /// ������
    /// Device Connector
    /// </summary>
    private UnityBleConnect connector = new UnityBleConnect(new UnityBleConfig());

    /// <summary>
    /// ָ����ֵ�����¼�
    /// Specify key value update events
    /// </summary>
    /// <param name="BWT901BLE"></param>
    public delegate void OnUpdateHandler(BWT901BLE BWT901BLE);

    public event OnUpdateHandler OnUpdate;

    /// <summary>
    /// �豸
    /// Device
    /// </summary>
    public BWT901BLE()
    {
        DeviceModel = new DeviceModel("50", "", new WitBleResolver(), new WitBleProcessor(), "61_0")
        {
            Connector = connector
        };
    }

    /// <summary>
    /// �����豸��ַ
    /// Set device address
    /// </summary>
    /// <param name="mac"></param>
    public void SetDeviceAddress(string mac) {
        connector.config.Mac = mac;
    }

    /// <summary>
    /// �����豸����
    /// Set device name
    /// </summary>
    /// <param name="name"></param>
    public void SetDeviceName(string name) {
        connector.config.DeviceName = name;
        DeviceModel.DeviceName = name;
    }

    /// <summary>
    /// ���豸
    /// Open device
    /// </summary>
    public void Open()
    {
        DeviceModel.OpenDevice();
        DeviceModel.OnListenKeyUpdate += DeviceModel_OnListenKeyUpdate;
    }

    /// <summary>
    /// �ر��豸
    /// Close device
    /// </summary>
    public void Close()
    {
        DeviceModel.CloseDevice();
        DeviceModel.OnListenKeyUpdate -= DeviceModel_OnListenKeyUpdate;
    }

    /// <summary>
    /// �Ƿ��
    /// Is it open
    /// </summary>
    /// <returns></returns>
    public bool IsOpen()
    {
        return DeviceModel.IsOpen;
    }

    /// <summary>
    /// ���������ݸ���ʱ��ص�����
    /// When sensor data is updated, it will be called back here
    /// </summary>
    /// <param name="deviceModel"></param>
    public void DeviceModel_OnListenKeyUpdate(DeviceModel deviceModel)
    {
        this.OnUpdate?.Invoke(this);
    }

    /// <summary>
    /// ����豸����
    /// Obtaining device data
    /// </summary>
    /// <param name="key"></param>
    /// <returns></returns>
    public string GetDeviceData(string key)
    {
        return DeviceModel.GetDeviceData(key);
    }

    /// <summary>
    /// ����豸����
    /// Obtaining device name
    /// </summary>
    /// <returns></returns>
    public string GetDeviceName()
    {
        return DeviceModel.DeviceName;
    }


    /// <summary>
    /// ���ٶ�У׼
    /// Acceleration calibration
    /// </summary>
    public void AppliedCalibration()
    {
        UnlockReg();
        Thread.Sleep(100);
        DeviceModel.SendData(new byte[5] { 255, 170, 1, 1, 0 });
        Thread.Sleep(5000);
        SaveReg();
    }

    public void StartFieldCalibration()
    {
        
    }

    public void EndFieldCalibration()
    {
        
    }


    public void SendData(byte[] data, out byte[] returnData, bool isWaitReturn = false, int waitTime = 100, int repetition = 1)
    {
        returnData = new byte[] { };
    }

    /// <summary>
    /// �������ݣ����ȴ��ظ�
    /// Send data without waiting for a reply
    /// </summary>
    /// <param name="data"></param>
    public void SendData(byte[] data) {
        DeviceModel.SendData(data);
    }

    /// <summary>
    /// ����Э������
    /// </summary>
    /// <param name="data"></param>
    public void SendProtocolData(byte[] data)
    {

    }

    /// <summary>
    /// ����Э������
    /// </summary>
    /// <param name="data"></param>
    /// <param name="waitTime"></param>
    public void SendProtocolData(byte[] data, int waitTime)
    {

    }

    /// <summary>
    /// ���ûش�����
    /// Set the return rate
    /// </summary>
    /// <param name="rate"></param>
    public void SetReturnRate(byte rate)
    {
        UnlockReg();
        Thread.Sleep(100);
        DeviceModel.SendData(new byte[5] { 255, 170, 3, rate, 0 });
        Thread.Sleep(100);
        SaveReg();
    }

    /// <summary>
    /// ����
    /// Save
    /// </summary>
    public void SaveReg()
    {
        DeviceModel.SendData(new byte[5] { 255, 170, 0, 0, 0 });
    }

    /// <summary>
    /// ����
    /// Unlock
    /// </summary>
    public void UnlockReg()
    {
        DeviceModel.SendData(new byte[5] { 255, 170, 105, 136, 181 });
    }
}