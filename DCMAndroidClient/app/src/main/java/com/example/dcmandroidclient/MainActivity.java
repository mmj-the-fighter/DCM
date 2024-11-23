package com.example.dcmandroidclient;

import android.content.Intent;
import android.content.SharedPreferences;
import android.os.Bundle;
import android.util.Log;
import android.view.View;
import android.widget.Button;
import android.widget.EditText;

import androidx.appcompat.app.AppCompatActivity;

import java.io.BufferedReader;
import java.io.IOException;
import java.io.Serializable;
import java.util.ArrayList;

import android.widget.Toast;


public class MainActivity extends AppCompatActivity {
    private EditText ipAddressEditText;
    private EditText portEditText;
    private Button connectButton;
    private String ipOnShPrefs;

    private ArrayList<String> ackStringList = new ArrayList<String>();

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);
        ipAddressEditText = findViewById(R.id.ipAddressEditText);
        //load ip address from shared preferences
        SharedPreferences sh = getSharedPreferences("DCMClientAndroidSP", MODE_PRIVATE);
        ipOnShPrefs = sh.getString("ipaddr", "");
        ipAddressEditText.setText(ipOnShPrefs);
        //
        portEditText = findViewById(R.id.portEditText);
        //
        connectButton = findViewById(R.id.connectButton);
        //
        connectButton.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                if (ConnectionDataSingleton.getInstance().isConnected()) {
                    Toast.makeText(getApplicationContext(), "Already Connected", Toast.LENGTH_LONG).show();
                    return;
                }
                if (ipAddressEditText.getText().toString().compareTo("") == 0) {
                    Toast.makeText(getApplicationContext(), "IP empty", Toast.LENGTH_LONG).show();
                    return;
                }
                if (portEditText.getText().toString().compareTo("") == 0) {
                    Toast.makeText(getApplicationContext(), "Port empty", Toast.LENGTH_LONG).show();
                    return;
                }
                new Thread(new ConnectTask()).start();
            }
        });

    }

    private class ConnectTask implements Runnable {

        @Override
        public void run() {
            String userEnteredIP = ipAddressEditText.getText().toString();
            int userEnteredPort = 64000;
            try {
                userEnteredPort = Integer.parseInt(portEditText.getText().toString());
            } catch (NumberFormatException e) {
                runOnUiThread(new Runnable() {
                    public void run() {
                        Toast.makeText(getApplicationContext(), "Invalid port num", Toast.LENGTH_LONG).show();
                    }
                });
                return;
            }

            boolean connFlag =
                    ConnectionDataSingleton.getInstance().connect(userEnteredIP, userEnteredPort);

            if (!connFlag) {
                runOnUiThread(new Runnable() {
                    public void run() {
                        Toast.makeText(getApplicationContext(), "Connect failed", Toast.LENGTH_LONG).show();
                    }
                });
                return;
            }

            //if connected on a different ip then update shared preferences
            if (ipOnShPrefs.compareTo(userEnteredIP) != 0) {
                SharedPreferences sharedPreferences = getSharedPreferences("DCMClientAndroidSP", MODE_PRIVATE);
                SharedPreferences.Editor myEdit = sharedPreferences.edit();
                myEdit.putString("ipaddr", userEnteredIP);
                myEdit.apply();
            }

            //Prepare acks array
            if (ConnectionDataSingleton.getInstance().isConnected()) {
                prepareAcknowledgementArrayList();
            }else{
                runOnUiThread(new Runnable() {
                    public void run() {
                        Toast.makeText(getApplicationContext(), "Didn't get ACK", Toast.LENGTH_LONG).show();
                    }
                });
                return;
            }

            //Send ack array to second activity
            Intent intent = new Intent(getApplicationContext(), SendCommandActivity.class);
            Bundle args = new Bundle();
            args.putSerializable("ARRAYLIST",(Serializable)ackStringList);
            intent.putExtra("BUNDLE",args);
            startActivity(intent);

        }
    }

    private void prepareAcknowledgementArrayList(){
        try
        {
            String response;
            BufferedReader reader = ConnectionDataSingleton.getInstance().getReader();
            while ((response = reader.readLine()) != null) {
                ackStringList.add(response);
                if (response.equals("done")) {
                    break;
                }
            }
        }
        catch(IOException e) {
            e.printStackTrace();
            Log.e("IO","IO"+e);
        }

    }

}