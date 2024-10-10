using System;
using System.Collections;
using System.Collections.Generic;
using UnityEngine;
using Wit.SDK.Modular.Sensor.Modular.DataProcessor.Constant;

public class Box : MonoBehaviour
{
    /// <summary>
    /// �豸ģ��
    /// </summary>
    BWT901BLE device;

    float BoxAngX = 0;
    float BoxAngY = 0;
    float BoxAngZ = 0;

    // Start is called before the first frame update
    void Start()
    {
        // �������豸�¼�
        Bwt901ble5UI.UpdateBox -= Bwt901ble5UI_Connect;
        Bwt901ble5UI.UpdateBox += Bwt901ble5UI_Connect;
    }

    private void Bwt901ble5UI_Connect(BWT901BLE bWT901BLE)
    {
        device = bWT901BLE;
        device.OnUpdate += Bwt901ble5UI_OnUpdate;
    }

    /// <summary>
    /// ָ����ֵˢ��ʱ
    /// </summary>
    /// <param name="BWT901BLE"></param>
    private void Bwt901ble5UI_OnUpdate(BWT901BLE BWT901BLE)
    {
        var AngX = BWT901BLE.GetDeviceData(WitSensorKey.AngleX);
        var AngY = BWT901BLE.GetDeviceData(WitSensorKey.AngleY);
        var AngZ = BWT901BLE.GetDeviceData(WitSensorKey.AngleZ);
        if (!string.IsNullOrEmpty(AngX) && !string.IsNullOrEmpty(AngY) && !string.IsNullOrEmpty(AngZ)) {
            BoxAngX = - (float.Parse(AngY));
            BoxAngY = float.Parse(AngZ);
            BoxAngZ = float.Parse(AngX);
        }
    }

    public float rotationSpeed = 50f; //������ת���ٶ�
    void Update()
    {
        if (device != null) {
            transform.rotation = Quaternion.Euler(BoxAngX, BoxAngY, BoxAngZ);
        }
    }
}
