package com.adventure.game;

import android.os.Bundle;
import android.os.Handler;
import android.os.Looper;
import android.text.TextUtils;
import android.text.method.ScrollingMovementMethod;
import android.view.View;
import android.widget.Button;
import android.widget.EditText;
import android.widget.ScrollView;
import android.widget.TextView;

import androidx.appcompat.app.AppCompatActivity;

import java.io.File;

public class GameActivity extends AppCompatActivity implements View.OnClickListener {
    
    static {
        System.loadLibrary("adventure-game");
    }
    
    private TextView outputText;
    private EditText inputText;
    private Button sendBtn;
    private ScrollView scrollView;
    
    private Handler handler = new Handler(Looper.getMainLooper());
    private boolean apiAvailable = false;
    
    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        
        try {
            setContentView(R.layout.activity_game);
        } catch (Exception e) {
            e.printStackTrace();
            return;
        }
        
        initViews();
        
        String modelPath = getIntent().getStringExtra("model_path");
        boolean gameInit = false;
        try {
            gameInit = initGame(modelPath);
        } catch (Exception e) {
            e.printStackTrace();
        }
        final boolean finalGameInit = gameInit;
        
        onGameOutput("正在启动...\n检查 API 服务器...\n");
        
        new Thread(() -> {
            boolean connected = false;
            String msg = "";
            try {
                connected = ApiClient.isServerRunning();
            } catch (Exception e) {
                msg = "检查出错：" + e.getMessage() + "\n";
            }
            
            final boolean finalConnected = connected;
            final String finalMsg = msg;
            runOnUiThread(() -> {
                StringBuilder sb = new StringBuilder();
                if (finalConnected) {
                    sb.append("✓ API 服务器已连接\n");
                    sb.append("输入 'ask [问题]' 测试 AI 对话\n");
                } else {
                    sb.append("✗ API 服务器未响应\n");
                    sb.append("请在 Termux 中启动:\n");
                    sb.append("./server -m your-model.gguf --host 0.0.0.0 --port 8080\n");
                    if (!finalMsg.isEmpty()) sb.append(finalMsg);
                }
                
                if (finalGameInit) {
                    sb.append("\n游戏已加载！输入 'help' 查看命令帮助\n");
                }
                
                onGameOutput(sb.toString());
            });
        }).start();
    }
    
    private void initViews() {
        outputText = findViewById(R.id.gameOutput);
        inputText = findViewById(R.id.gameInput);
        sendBtn = findViewById(R.id.sendBtn);
        scrollView = findViewById(R.id.scrollView);
        
        outputText.setMovementMethod(new ScrollingMovementMethod());
        sendBtn.setOnClickListener(this);
        
        inputText.setOnEditorActionListener((v, actionId, event) -> {
            sendMessage();
            return true;
        });
    }
    
    @Override
    public void onClick(View v) {
        sendMessage();
    }
    
    private void sendMessage() {
        String input = inputText.getText().toString().trim();
        if (TextUtils.isEmpty(input)) {
            return;
        }
        
        inputText.setText("");
        onGameOutput("> " + input);
        
        if (input.startsWith("ask ")) {
            handleAskCommand(input.substring(4));
        } else {
            String response = processInput(input);
            onGameOutput(response);
        }
    }
    
    private void handleAskCommand(String question) {
        onGameOutput("\n思考中...\n");
        
        new Thread(() -> {
            String response = ApiClient.generateResponse(question, 256);
            runOnUiThread(() -> {
                onGameOutput("AI: " + response + "\n\n");
            });
        }).start();
    }
    
    public void onGameOutput(final String text) {
        runOnUiThread(() -> {
            if (!TextUtils.isEmpty(text)) {
                outputText.append(text);
                if (!text.endsWith("\n")) {
                    outputText.append("\n");
                }
                scrollView.post(() -> {
                    scrollView.fullScroll(View.FOCUS_DOWN);
                });
            }
        });
    }
    
    @Override
    protected void onDestroy() {
        super.onDestroy();
        cleanupGame();
    }
    
    // JNI 方法
    private native boolean initGame(String modelPath);
    private native String processInput(String input);
    private native void cleanupGame();
    private native boolean isGameRunning();
    private native String getCurrentScene();
    private native int getGold();
}
