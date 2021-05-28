# Software Report

## Concept Development

The Cloud Architecture for this project will be done through using AWS services like Kinesis, S3, and API Gateway. AWS Kinesis allows for real-time data streams from the device at any throughput for analytics and storage. AWS S3, which stands for Simple Storage Service, is used for cheaply storing data over a period of time. AWS API Gateway will then be used as the REST API to retrieve the data from the S3 bucket to be sent to the mobile application 

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


## Mobile Application

Application is built using React-Native to develop the mobile application. Amazon Web Services (AWS) for the storage and computation of the data. Sensors to calculate the values of different gases present in the environment.





 <p align ="center">  <img width="300" src="/images/image13.jpg" alt="login"><p>

<p align = "center">Figure 2: Login Screen <p>


<p align = "center"> This screen allows the user to login to the application, register new users, or reset their account password. <p>

 <p align ="center">  <img width="300" src="/images/image12.jpg" alt="login"><p>

<p align = "center">Figure 3: Data Visualizations <p>

<p align = "center"> The Home screen contains the main Dashboard where the users can see the Air Quality History for the last seven days or a week as a histogram that maps the AQI and displays it. The Y-axis of the histogram’s Y-axis represents the percentage of the AQI and the X-axis display the days of the week.  Above the histogram, we have displayed the current sensors data and the date.<p>

 <p align ="center">  <img width="300" src="/images/image12.jpg" alt="login"><p>


<p align = "center">Figure 4: Room Screen<p>


<p align = "center"> Rooms: The room screen displays the room’s e real-time air quality data using the pie charts uploaded to AWS by the sensors. The pie-charts are color-coded, and the percentages displayed on the charts are mapped to the values provided by the sensors. <p>



 <p align ="center">  <img width="300" src="/images/image11.jpg" alt="login"> <img width="300" src="/images/image9.jpg" alt="login"><p>

<p align = "center">Figure 5 and 6: Working Camera with QR Scan<p>

<p align = "center">The two above screenshots (Figure 11 and Figure 12) of our Applications’ User Interface display both a working camera and the ability to do a QR scan. <p>
