
# mqtt-daemon
===========================
####目录结构描述  
├── Readme.md            //帮助文档  
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
    └── test-config.h     --------------  示例代码主要配置   
 
#### 使用流程介绍   
   
##### 配置  
   编辑test-config.h，完成您的基本配置  

##### 编译     
   进入到主目录   
   `cd mqtt-daemon`    
   创建build 目录   
   `mkdir build`  
   进入build 目录创建cmake 工程  
   `cd build`  
   `cmake .. `  
   完成编译  
   `make`  

##### 运行调试  
   在build目录下会有一个bin 文件夹，进入，执行 `./MqttSensor` 即可完成平台连接和数据上传  
