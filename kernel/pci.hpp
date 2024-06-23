// PCIバス制御のプログラム

#pragma once

#include <cstdint>
#include <array>

#include "error.hpp"

namespace pci {
    //configアドレス
    const uint16_t kConfigAddress = 0x0cf8; //CONFIG_ADDRESSレジスタのIOポートアドレス
    const uint16_t kConfigData=0x0cfc; //CONFIG_DATAレジスタのIOポートアドレス

    struct ClassCode{
        uint8_t base, sub, interface;

        bool Match(uint8_t b) {return b==base;}
        bool Match(uint8_t b, uint8_t s) {return Match(b)&& s==sub;}
        bool Match(uint8_t b, uint8_t s, uint8_t i) {return Match(b, s)&& i==interface;}
    };

    // PCIデバイスを操作するための基礎データを格納
    struct Device {
        uint8_t bus, device, function, header_type;
        ClassCode class_code;
    };

    //データの読み書き
    void WriteAddress(uint32_t address); //CONFIG_ADDRESSに整数を書き込む
    void WriteData(uint32_t value); //CONFIG_DATAに整数を書き込む
    uint32_t ReadData(); //CONFIG_DATAから整数を読み込む

    uint16_t ReadVendorId(uint8_t bus, uint8_t device, uint8_t function);
    uint16_t ReadDeviceId(uint8_t bus, uint8_t device, uint8_t function);
    uint8_t ReadHeaderType(uint8_t bus, uint8_t device, uint8_t function);
    ClassCode ReadClassCode(uint8_t bus, uint8_t device, uint8_t function); //31-24: ベースクラス2, 23-16:サブクラス
    uint32_t ReadBusNumbers(uint8_t bus, uint8_t device, uint8_t function); //23-16:サブオーディネイトパス番号
    
    inline uint16_t ReadVendorId(const Device& dev){
        return ReadVendorId(dev.bus, dev.device, dev.function);
    }

    uint32_t ReadConfReg(const Device& dev, uint8_t reg_addr);

    void WriteConfReg(const Device& dev, uint8_t reg_addr, uint32_t value);

    bool IsSingleFunctionDevice(uint8_t header_type);

    
    inline std::array<Device, 32> devices; //ScanAllBus()により発見されたPCIデバイスの一覧
    inline int num_devices; //deviceの有効な要素の数

    //PCIデバイスをバス0から再帰的に全て探索して、devicesの先頭から書き込む
    // 発見したデバイスの数を num_deviceに設定する
    Error ScanAllBus(); 

    constexpr uint8_t CalcBarAddress(unsigned int bar_index){
        return 0x10 + 4*bar_index;
    }

    WithError<uint64_t> ReadBar(Device& device, unsigned int bar_index);
}