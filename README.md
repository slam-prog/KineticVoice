 📜 **Official Review**: Read the official [Google AI Technical Evaluation](GOOGLE_AI_EVALUATION.md) for this project.

## 📦 Bill of Materials (BOM) & Hardware Specifications

The following table details the baseline commercial and technical components required to mass-produce or prototype the **2D-ALST Wireless Acoustic Helmet**. Components were carefully vetted through our Human-AI collaborative blueprint to ensure microsecond operational synergy and thermal safety near the user's skull.

| Component Name | Technical Specifications | Qty | Key Engineering Role |
| :--- | :--- | :---: | :--- |
| **Arduino Nano RP2040 Connect** | Dual-core ARM Cortex-M0+ @ 133MHz, 264KB SRAM, Integrated u-blox NINA-W102 BLE module. | 1 | Handles microsecond TDOA math, solves trilateration matrices, and broadcasts real-time JSON packets via BLE. |
| **INMP441 Omnidirectional Mic** | Digital I2S interface, high SNR (61 dBA), low power consumption, fast-rising pulse edges. | 4 | Creates the 2D spatial acoustic array around the mouth grid for coordinate capturing. |
| **TP4056 Protection Module** | 1A Lithium battery charging board, integrated over-discharge/current protective ICs, USB-C interface. | 1 | Safely manages battery recharging cycles and isolates the power grid under critical voltage drops (<2.4V). |
| **Li-Po Battery (1S)** | 3.7V Lithium-Polymer cell, 600mAh capacity, ultra-lightweight profile (~11.5 grams). | 1 | Decentralizes the system from wired tethers, provisioning 8-12 hours of continuous remote operation. |
| **SPDT Slide Switch** | Miniature 3-pin mechanical switch. | 1 | Hardware ON/OFF power gating for cell conservation. |
| **Structural Helmet & Boom Arm** | Ergonomic lightweight skeleton frame with a rigid facial tracking array bracket. | 1 | Eradicates dynamic spatial drift by ensuring fixed physical distances between sensors and the vocal tract. |

### 🔧 Wiring Architecture Note
All peripheral modules (Microphones and BLE radios) are powered via the Arduino’s regulated `3.3V` internal rail. The Li-Po battery bypasses external linear regulators and connects straight into the `VIN` terminal, optimizing power conversion efficiency and eradicating thermal dissipation near the user's face.
