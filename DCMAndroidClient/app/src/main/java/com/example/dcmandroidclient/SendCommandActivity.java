package com.example.dcmandroidclient;


import android.os.Bundle;
import android.util.Log;
import android.view.KeyEvent;
import android.view.View;
import android.widget.AdapterView;
import android.widget.ArrayAdapter;
import android.widget.Button;
import android.widget.EditText;
import android.widget.ListView;

import androidx.appcompat.app.AppCompatActivity;

import java.io.BufferedReader;
import java.io.IOException;

import android.content.Intent;

import android.widget.Toast;

import java.io.Serializable;
import java.util.ArrayList;


public class SendCommandActivity extends AppCompatActivity {

    private ListView chatListView;
    private ArrayAdapter<String> chatAdapter;
    private EditText commandEditText;
    private Button sendButton;


    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_command_sender);

        Intent intent = getIntent();
        Bundle args = intent.getBundleExtra("BUNDLE");
        ArrayList<String> ackStringList =
                (ArrayList<String>) args.getSerializable("ARRAYLIST");

        commandEditText = findViewById(R.id.commandEditText);
        sendButton = findViewById(R.id.sendButton);
        chatListView = findViewById(R.id.chatListView);
        chatAdapter = new ArrayAdapter<>(this, android.R.layout.simple_list_item_1);
        chatListView.setAdapter(chatAdapter);

        for( String s : ackStringList){
            chatAdapter.add(s);
        }

        sendButton.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                if (!ConnectionDataSingleton.getInstance().isConnected()) {
                    Toast.makeText(getApplicationContext(), "Not Connected", Toast.LENGTH_LONG).show();
                    return;
                }
                String command = commandEditText.getText().toString();
                if (command.compareTo("") == 0) {
                    Toast.makeText(getApplicationContext(), "Empty Command", Toast.LENGTH_LONG).show();
                    return;
                }
                new Thread(new SendTask(command)).start();
            }
        });

        chatListView.setOnItemClickListener(new AdapterView.OnItemClickListener() {
            @Override
            public void onItemClick(AdapterView<?> parent, View view, int position, long id) {
                if (!ConnectionDataSingleton.getInstance().isConnected()) {
                    Toast.makeText(getApplicationContext(), "Not Connected", Toast.LENGTH_LONG).show();
                    return;
                }
                String command = chatAdapter.getItem(position);
                commandEditText.setText(command);
                //new Thread(new SendTask(command)).start();
            }
        });
    }


    @Override
    public boolean onKeyDown(int keyCode, KeyEvent event) {
        if (keyCode == KeyEvent.KEYCODE_BACK) {
            if (ConnectionDataSingleton.getInstance().isConnected()) {
                String command = "done";
                new Thread(new SendTask(command)).start();
            }
            Intent intent = new Intent(getApplicationContext(), MainActivity.class);
            intent.addFlags(Intent.FLAG_ACTIVITY_CLEAR_TOP);
            startActivity(intent);
            return true;
        }
        return super.onKeyDown(keyCode, event);
    }


    private class SendTask implements Runnable {
        private String command;

        public SendTask(String command) {
            this.command = command;
        }

        @Override
        public void run() {
            try {
                ConnectionDataSingleton.getInstance().getWriter().println(command);
                runOnUiThread(new Runnable() {
                    @Override
                    public void run() {
                        chatAdapter.add(command);
                    }
                });

                if (command.compareTo("done") == 0) {
                    ConnectionDataSingleton.getInstance().disconnect();
                    return;
                }

                String response = ConnectionDataSingleton.getInstance().getReader().readLine();
                final String finalResponse = response;
                runOnUiThread(new Runnable() {
                    @Override
                    public void run() {
                        chatAdapter.add(finalResponse);
                    }
                });


            } catch (IOException e) {
                e.printStackTrace();
            }

            runOnUiThread(new Runnable() {
                @Override
                public void run() {
                    commandEditText.setText("");
                }
            });
        }
    }
}