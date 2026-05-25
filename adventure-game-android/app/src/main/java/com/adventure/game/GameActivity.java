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

public class GameActivity extends AppCompatActivity implements View.OnClickListener {
    
    static {
        System.loadLibrary("adventure-game");
    }
    
    private TextView outputText;
    private EditText inputText;
    private Button sendBtn;
    private ScrollView scrollView;
    
    private Handler handler = new Handler(Looper.getMainLooper());
    
    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_game);
        
        initViews();
        
        String modelPath = getIntent().getStringExtra("model_path");
        initGame(modelPath);
        
        // 显示初始消息
        onGameOutput("游戏已加载！输入 'help' 查看命令帮助\n");
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
        processInput(input);
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
