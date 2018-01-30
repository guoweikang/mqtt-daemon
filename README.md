# mqtt-daemon
mqtt 示例代码
代码目录介绍: 
.
├── CMakeLists.txt----------------------工程cmake

├── mqtt ------------------------------ One Net mqtt sdk 代码头文件
│   ├── cJSON.h
│   ├── config.h
│   ├── mqtt_base.hpp
│   ├── mqtt_buffer.h
│   ├── mqtt_buf.hpp
│   └── mqtt.h
├── README.md
├── sdk---------------------------------One Net  mqtt sdk 代码源文件
│   ├── cJSON.c
│   ├── CMakeLists.txt
│   ├── mqtt_buffer.c
│   └── mqtt.c
└── src--- -----------------------------mqtt sdk 使用代码示例
    ├── CMakeLists.txt
    ├── common.c
    ├── common.h
    ├── control.c
    ├── sensor.c
    └── test-config.h   --------------  示例代码主要配置
