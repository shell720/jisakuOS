// PCIバス制御のプログラム

#pragma once

#include <cstdint>
#include <array>

#include "error.hpp"

namespace pci {
    //configアドレス
    const uint16_t kConfigAddress = 0x0cf8; //CONFIG_ADDRESSレジスタのIOポートアドレス
    const uint16_t kConfigData=0x0cfc; //CONFIG_DATAレジスタのIOポートアドレス

    //データの読み書き
    void WriteAddress(uint32_t address); //CONFIG_ADDRESSに整数を書き込む
    void WriteData(uint32_t value); //CONFIG_DATAに整数を書き込む
    uint32_t ReadData(); //CONFIG_DATAから整数を読み込む

    uint16_t ReadVendorId(uint8_t bus, uint8_t device, uint8_t function);
    uint16_t ReadDeviceId(uint8_t bus, uint8_t device, uint8_t function);
    uint8_t ReadHeaderType(uint8_t bus, uint8_t device, uint8_t function);
    uint32_t ReadClassCode(uint8_t bus, uint8_t device, uint8_t function); //31-24: ベースクラス2, 23-16:サブクラス
    uint32_t ReadBusNumbers(uint8_t bus, uint8_t device, uint8_t function); //23-16:サブオーディネイトパス番号
    
    bool IsSingleFunctionDevice(uint8_t header_type);

    // PCIデバイスを操作するための基礎データを格納
    struct Device {
        uint8_t bus, device, function, header_type;
    };

    inline std::array<Device, 32> devices; //ScanAllBus()により発見されたPCIデバイスの一覧
    inline int num_devices; //deviceの有効な要素の数

    //PCIデバイスをバス0から再帰的に全て探索して、devicesの先頭から書き込む
    // 発見したデバイスの数を num_deviceに設定する
    Error ScanAllBus(); 
}