/**
 * Created by K. Suwatchai (Mobizt)
 *
 * Email: k_suwatchai@hotmail.com
 *
 * Github: https://github.com/mobizt/Firebase-ESP-Client
 *
 * Copyright (c) 2022 mobizt
 *
 */

// This example shows how to create a document in a document collection. This operation required Email/password, custom or OAUth2.0 authentication.

#if defined(ESP32)
#include <WiFi.h>
#elif defined(ESP8266)
//#include <ESP8266WiFi.h>
#endif

#include <Firebase_ESP_Client.h>

// Provide the token generation process info.
#include <addons/TokenHelper.h>

/* 1. Define the WiFi credentials */
#define WIFI_SSID "TechPublic"
#define WIFI_PASSWORD ""

/* 2. Define the API Key */
#define API_KEY "AIzaSyAiE9hg0wrGLOPtctSv81nD_3mZnHOFWqI"

/* 3. Define the project ID */
#define FIREBASE_PROJECT_ID "aid-scale"

/* 4. Define the user Email and password that alreadey registerd or added in your project */
#define USER_EMAIL "ahmad@1.com"
#define USER_PASSWORD "123456"

// Define Firebase Data object
FirebaseData fbdo;

FirebaseAuth auth;
FirebaseConfig config;

unsigned long dataMillis = 0;
int count = 0;

// The Firestore payload upload callback function
void fcsUploadCallback(CFS_UploadStatusInfo info)
{
    if (info.status == fb_esp_cfs_upload_status_init)
    {
        Serial.printf("\nUploading data (%d)...\n", info.size);
    }
    else if (info.status == fb_esp_cfs_upload_status_upload)
    {
        Serial.printf("Uploaded %d%s\n", (int)info.progress, "%");
    }
    else if (info.status == fb_esp_cfs_upload_status_complete)
    {
        Serial.println("Upload completed ");
    }
    else if (info.status == fb_esp_cfs_upload_status_process_response)
    {
        Serial.print("Processing the response... ");
    }
    else if (info.status == fb_esp_cfs_upload_status_error)
    {
        Serial.printf("Upload failed, %s\n", info.errorMsg.c_str());
    }
}

void setup()
{

    Serial.begin(115200);

    WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
    Serial.print("Connecting to Wi-Fi");
    while (WiFi.status() != WL_CONNECTED)
    {
        Serial.print(".");
        delay(300);
    }
    Serial.println();
    Serial.print("Connected with IP: ");
    Serial.println(WiFi.localIP());
    Serial.println();

    Serial.printf("Firebase Client v%s\n\n", FIREBASE_CLIENT_VERSION);

    /* Assign the api key (required) */
    config.api_key = API_KEY;

    /* Assign the user sign in credentials */
    auth.user.email = USER_EMAIL;
    auth.user.password = USER_PASSWORD;

    /* Assign the callback function for the long running token generation task */
    config.token_status_callback = tokenStatusCallback; // see addons/TokenHelper.h

    Firebase.begin(&config, &auth);

    Firebase.reconnectWiFi(true);

    // For sending payload callback
    // config.cfs.upload_callback = fcsUploadCallback;
}

void loop()
{

    // Firebase.ready() should be called repeatedly to handle authentication tasks.

//    if (Firebase.ready() && (millis() - dataMillis > 60000 || dataMillis == 0))
//    {
//        dataMillis = millis();
//
//        // For the usage of FirebaseJson, see examples/FirebaseJson/BasicUsage/Create.ino
//        FirebaseJson content;
//
//        // We will create the nested document in the parent path "a0/b0/c0
//        // a0 is the collection id, b0 is the document id in collection a0 and c0 is the collection id in the document b0.
//        // and d? is the document id in the document collection id c0 which we will create.
//        String documentPath = "test/3331" + String(count);
//
//        // If the document path contains space e.g. "a b c/d e f"
//        // It should encode the space as %20 then the path will be "a%20b%20c/d%20e%20f"
//
//        // double
//        //content.set("fields/myDouble/doubleValue", 123.45678);
//
//        // boolean
//       // content.set("fields/myBool/booleanValue", true);
//
////        // language
////        content.set("fields/language/stribgValue", "HE");
//
//        // name
//       // content.set("fields/name/nullValue"); // no value set
//
//        String doc_path = "projects/";
//        doc_path += FIREBASE_PROJECT_ID;
//        doc_path += "/databases/(default)/documents/coll_id/doc_id"; // coll_id and doc_id are your collection id and document id
//
////        // reference
////        content.set("fields/myRef/referenceValue", doc_path.c_str());
//
//        // timestamp
//      //  content.set("fields/myTimestamp/timestampValue", "2014-10-02T15:01:23Z"); // RFC3339 UTC "Zulu" format
//
////        // bytes
////        content.set("fields/myBytes/bytesValue", "aGVsbG8="); // base64 encoded
//
//        // array
//        content.set("fields/myArray/arrayValue/values/[0]/stringValue", "test");
//        content.set("fields/myArray/arrayValue/values/[1]/integerValue", "1000");
//        content.set("fields/myArray/arrayValue/values/[2]/booleanValue", true);
//
//        // map
////        content.set("fields/myMap/mapValue/fields/name/stringValue", "wrench");
////        content.set("fields/myMap/mapValue/fields/mass/stringValue", "1.3kg");
////        content.set("fields/myMap/mapValue/fields/count/integerValue", "3");
//
////        // lat long
////        content.set("fields/myLatLng/geoPointValue/latitude", 1.486284);
////        content.set("fields/myLatLng/geoPointValue/longitude", 23.678198);
//
//        //count++;
//        //JsonArray list = ["1","2","3"];
//        //content.add(21);
//        FirebaseJsonArray arr;
//        arr.add("banana");
//        //content.add("add");
//        Serial.print("Create a document... ");
//        if (Firebase.Firestore.commitDocument(&fbdo, FIREBASE_PROJECT_ID, "" /* databaseId can be (default) or empty */, documentPath.c_str(), content.raw()))
//            Serial.printf("ok\n%s\n\n", fbdo.payload().c_str());
//        else
//            Serial.println(fbdo.errorReason());
//    }


if (Firebase.ready() && (millis() - dataMillis > 60000 || dataMillis == 0))
    {
        dataMillis = millis();
        count++;

        Serial.print("Commit a document (append array)... ");

        // The dyamic array of write object fb_esp_firestore_document_write_t.
        std::vector<struct fb_esp_firestore_document_write_t> writes;

        // A write object that will be written to the document.
        struct fb_esp_firestore_document_write_t transform_write;

        // Set the write object write operation type.
        // fb_esp_firestore_document_write_type_update,
        // fb_esp_firestore_document_write_type_delete,
        // fb_esp_firestore_document_write_type_transform
        transform_write.type = fb_esp_firestore_document_write_type_transform;

        // Set the document path of document to write (transform)
        transform_write.document_transform.transform_document_path = "Jobs/5.25.2022";

        // Set a transformation of a field of the document.
        struct fb_esp_firestore_document_write_field_transforms_t field_transforms;

        // Set field path to write.
        field_transforms.fieldPath = "jobs";

        // Set the transformation type.
        // fb_esp_firestore_transform_type_set_to_server_value,
        // fb_esp_firestore_transform_type_increment,
        // fb_esp_firestore_transform_type_maaximum,
        // fb_esp_firestore_transform_type_minimum,
        // fb_esp_firestore_transform_type_append_missing_elements,
        // fb_esp_firestore_transform_type_remove_all_from_array
        field_transforms.transform_type = fb_esp_firestore_transform_type_append_missing_elements;

        // For the usage of FirebaseJson, see examples/FirebaseJson/BasicUsage/Create.ino
        FirebaseJson content;

        //String txt = "Hello World! " + String(count);
        content.set("values/[0]/mapValue/fields/Name/stringValue", "wrench");
        content.set("values/[0]/mapValue/fields/ID/stringValue", "3132");
        content.set("values/[0]/mapValue/fields/SubmitTime/stringValue", "12:12");
        content.set("values/[0]/mapValue/fields/Tolerance/doubleValue", 3.2);
        content.set("values/[0]/mapValue/fields/Weight/doubleValue", 10.3);
        content.set("values/[0]/mapValue/fields/WorkTime/doubleValue", 5);
        content.set("values/[0]/mapValue/fields/ProductName/stringValue", "p4");
        
        // Set the transformation content.
        field_transforms.transform_content = content.raw();

        // Add a field transformation object to a write object.
        transform_write.document_transform.field_transforms.push_back(field_transforms);

        // Add a write object to a write array.
        writes.push_back(transform_write);

        if (Firebase.Firestore.commitDocument(&fbdo, FIREBASE_PROJECT_ID, "" /* databaseId can be (default) or empty */, writes /* dynamic array of fb_esp_firestore_document_write_t */, "" /* transaction */))
            Serial.printf("ok\n%s\n\n", fbdo.payload().c_str());
        else
            Serial.println(fbdo.errorReason());
    }


}
