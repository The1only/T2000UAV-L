using System;
using Wit.SDK.Modular.Sensor.Modular.Connector.Entity;
using Wit.SDK.Modular.Sensor.Modular.Connector.Interface;
using Wit.SDK.Sensor.Connector.Entity;

namespace Wit.SDK.Sensor.Connector.Role 
{
    /// <summary>
    /// unity����������
    /// </summary>
    public class UnityBleConnect : IConnector
    {
        /// <summary>
        /// ����������
        /// </summary>
        public UnityBleConfig config = new UnityBleConfig();

        /// <summary>
        /// ����
        /// </summary>
        /// <param name="config"></param>
        public UnityBleConnect(UnityBleConfig config) {
            this.config = config;
        }

        /// <summary>
        /// �������
        /// </summary>
        public override void CheckConfig()
        {
            if (config.Mac == null)
            {
                throw new Exception("δ���� Mac ��ַ");
            }

            if (config.ServiceGuid == null)
            {
                throw new Exception("δ���� ServiceGuid");
            }

            if (config.WriteGuid == null)
            {
                throw new Exception("δ���� WriteGuid");
            }

            if (config.NotifyGuid == null)
            {
                throw new Exception("δ���� NotifyGuid");
            }
        }

        public override void Close()
        {
            if (ConnectStatus == ConnectStatus.Closed) {
                return;
            }
            // disconnect
            BluetoothLEHardwareInterface.UnSubscribeCharacteristic(config.Mac, config.ServiceGuid, config.NotifyGuid, (characteristic) => {
                BluetoothLEHardwareInterface.DisconnectPeripheral(config.Mac, (disconnectAddress) => {
                    // ��־Ϊ�ر�״̬
                    ConnectStatus = ConnectStatus.Closed;
                });
            });
        }

        /// <summary>
        /// �������
        /// </summary>
        /// <returns></returns>
        public override IConnectConfig GetConfig()
        {
            return config;
        }

        public override void Open()
        {
            if (ConnectStatus == ConnectStatus.Opened) 
            {
                return;
            }

            CheckConfig();

            BluetoothLEHardwareInterface.ConnectToPeripheral(config.Mac, (address) => {}, null, (address, service, characteristic) => {
                ConnectStatus = ConnectStatus.Opened;
                // �ҷ���
                subscribe();
            }, null);
        }

        /// <summary>
        /// �ҷ���
        /// </summary>
        private void subscribe()
        {
            BluetoothLEHardwareInterface.SubscribeCharacteristic(config.Mac, config.ServiceGuid, config.NotifyGuid, null, (characteristic, bytes) => {
                onReceive(bytes);
            });
        }

        public override void SendData(byte[] data)
        {
            BluetoothLEHardwareInterface.WriteCharacteristic(config.Mac, config.ServiceGuid, config.WriteGuid, data, data.Length, false, (characteristic) =>{ });
        }
    }

}