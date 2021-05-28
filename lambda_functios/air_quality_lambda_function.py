import json
import boto3
from decimal import Decimal
from random import seed
from random import randint

s3 = boto3.resource('s3')


def lambda_handler(event, context):
   
   
    s2 = boto3.client('s3')
    obs = []
    resp = s2.list_objects_v2(Bucket='mkr1000v2')
    for obj in resp['Contents']:
      obs.append(obj['Key'])
    
    
    
    #print(*obs, sep = "\n")
    

    
    length_obs = len(obs) - 1
    #print(obs[length])
    
    #gets object from S3 bucket. Needs bucket name and object key
    #data = s3.Object('airquality-zimlim','MKRDataStream-1-2021-02-25-18-33-39-c7179194-2567-4cb2-a9ae-272aa6b01356')
    object_key = obs[length_obs]
    s3_name = 'mkr1000v2'
    data = s3.Object(s3_name,object_key)
    print(type(data))
    
    #parse thru the object and converts it to byte format
    body = data.get()['Body'].read()
    
    #converts bytes to string
    s = body.decode('utf8')
    
    index = len(s) - 1
    if len(s) > index:
        s = s[0 : index : ] + s[index + 1 : :]
    
    #adds square brackets so JSON.loads can read it and stores the data in a dictionary
    s = '[' + s + ']'
    print(s)
    fin_data = json.loads(s)
    length = len(fin_data)
   
    rand = randint(0,20)
    #val = fin_data[rand]
    rand_temp = fin_data[rand]['Temperature']
    rand_ppm = fin_data[rand]['DustDensity']
    rand_mq2 = fin_data[rand]['MQ2']
    rand_mq5 = fin_data[rand]['MQ5']
    rand_mq9 = fin_data[rand]['MQ9']
    rand_mq135 = fin_data[rand]['MQ135']
    rand_humidity = fin_data[rand]['Humidity']
    
    rand_data_dictionary = {}
    rand_data_dictionary['Temp'] = rand_temp
    rand_data_dictionary['PPM'] = rand_ppm
    rand_data_dictionary['MQ2'] = rand_mq2
    rand_data_dictionary['MQ5'] = rand_mq5
    rand_data_dictionary['MQ9'] = rand_mq9
    rand_data_dictionary['MQ135'] = rand_mq135
    rand_data_dictionary['Humidity'] = rand_humidity
    
    
    
    
    # ave_temp = 0
    # ave_ppm = 0
    # #parse through all the dictionary
    # for l in range(length):
    #     print('- ' * 20)
    #     l1 = fin_data[l]
    #     # Dont need this part except for sanity checks
    #     # print(l1)
    #     # print("Time: ")
    #     # print(l1['Time'])
    #     # print("Temperature: ")
    #     # print(l1['temperature'])
    #     # print("PPM: ")
    #     # print(l1['ppm'])
        
        
    #     temp = Decimal(fin_data[l]['temperature'])
    #     ppm = Decimal(fin_data[l]['ppm'])
        
    #     #comment out once data points are included in S3 bucket
    #     #dust = fin_data[l]['dust']
    #     #oxygen = fin_data['oxygen']
    #     #co2 = fin_data['co2']
    #     #aqi = fin_data['aqi']
        
    #     ave_temp = ave_temp + l1['temperature']
    #     ave_ppm = ave_ppm + l1['ppm']
        
    #     #comment out once data points are included in S3 bucket
    #     # ave_oxy = ave_oxy + l1['oxy']
    #     # ave_co2 = ave_co2 + l1['co2']
    #     # ave_aqi = ave_aqi + l1['aqi']
    #     # ave_dust = ave_dust + l1['dust']
        
    
    # ave_temp = ave_temp/length
    # ave_ppm = ave_ppm/length
    # # ave_oxy = ave_oxy/length
    # # ave_co2 = ave_co2/ length
    # # ave_aqi = ave_aqi/length
    # # ave_dust = ave_dust/length
    # print('- ' *20)
    
    
    
    return{
        'statusCode' : 200,
        'body': json.dumps(rand_data_dictionary)
        
    }
    
