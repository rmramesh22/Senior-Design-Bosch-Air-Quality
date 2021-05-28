## Concept Development

For this project I developed the Cloud Architecture for this project will be done through using AWS services like Kinesis, S3, and API Gateway. AWS Kinesis allows for real-time data streams from the device at any throughput for analytics and storage. AWS S3, which stands for Simple Storage Service, is used for cheaply storing data over a period of time. AWS API Gateway will then be used as the REST API to retrieve the data from the S3 bucket to be sent to the mobile application 

Once the data is processed and stored in AWS S3, API Gateway will be used to trigger a lambda function to pull the data from the S3 Bucket in JSON format. The JSON file will then be parsed and have the data extracted from it and be stored locally in the smartphone. 


## Cloud Architecture

![image](/images/cloud.png)


 <p align ="center"> Figure 1:  Cloud Architecture Topology and Service Components </p>


The above diagram (Figure 1) depicts the configuration of the cloud infrastructure and our choice of service components. Specifically, this includes the IoT device data formatting and transmission, the telemetry data ingestion and storage on the cloud and REST API for the mobile application to visualize the telemetry. The characteristics of the service components used in the architecture are as follows:

- AWS IoT Core: Acts as the gateway and facilitates the MQTT protocol-based communication between the devices and the AWS Cloud services. This service enables us to collect, store, and analyze the telemetry data received via MQTT protocol from multiple devices. It provides registration and authentication of devices using CSR certificates. It also facilitates a rules engine that triggers actions to transform the data and deliver it to AWS Kinesis Firehose and AWS S3 storage.

- AWS Kinesis Firehose:  This service helps us to capture, transform, and deliver the streaming data from the AWS IoT Core to AWS S3 storage. 

- AWS S3:  This service is used to provide persistent object storage for all the AWS IoT Core’s streaming data.

- AWS Lambda Functions: This managed service waits for pre-defined triggers via API Gateway to execute a specific command (retrieve data from storage, calculating AQI score, etc.) for delivering real-time data to the mobile application.
Amazon API Gateway: It acts as a REST API and proxy to our AWS Lambda functions and delivers real-time data to our Mobile application.

-AWS API Gateway : This service makes transmitting data to the mobile device extermely easy and secure. There was no need to know all the data destination beforehand since the client-side will need to make an HTTP request first. After that happens, the data will automatically be sent to the client given that they have the authorization key.

Each of these services are interconnected in the AWS pipeline with each other to allow telemetry data to be passed from the Arduino through the AWS Cloud to our mobile client application. It is important to ensure in this process that data is secure and is traveling efficiently to the client application for visualization. 

