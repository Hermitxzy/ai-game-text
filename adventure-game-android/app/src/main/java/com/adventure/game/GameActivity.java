package com.adventure.game;

import android.os.Bundle;
import android.os.Handler;
import android.os.Looper;
import android.text.TextUtils;
import android.text.method.ScrollingMovementMethod;
import android.view.View;
import android.widget.Button;
import android.widget.EditText;
import android.widget.HorizontalScrollView;
import android.widget.LinearLayout;
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
    private Button prevBtn;
    private Button nextBtn;
    private TextView commandText;
    private LinearLayout targetContainer;
    
    private Handler handler = new Handler(Looper.getMainLooper());
    private boolean apiAvailable = false;
    
    private String[] availableCommands = {};
    private int currentCommandIndex = -1;
    private String[] availableTargets = {};
    private int currentTargetIndex = -1;
    
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
        loadAvailableCommands();
        
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
                    sb.append("\n游戏已加载！使用 ← → 按钮或输入命令\n");
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
        prevBtn = findViewById(R.id.prevBtn);
        nextBtn = findViewById(R.id.nextBtn);
        commandText = findViewById(R.id.commandText);
        targetContainer = findViewById(R.id.targetContainer);
        
        outputText.setMovementMethod(new ScrollingMovementMethod());
        sendBtn.setOnClickListener(this);
        prevBtn.setOnClickListener(this);
        nextBtn.setOnClickListener(this);
        
        inputText.setOnEditorActionListener((v, actionId, event) -> {
            sendMessage();
            return true;
        });
        
        updateCommandText();
        updateTargetButtons();
    }
    
    private void loadAvailableCommands() {
        String commands = getAvailableCommands();
        if (!TextUtils.isEmpty(commands)) {
            availableCommands = commands.split("\\|");
            currentCommandIndex = 0;
            updateCommandText();
        }
    }
    
    private void updateCommandText() {
        if (currentCommandIndex >= 0 && currentCommandIndex < availableCommands.length) {
            commandText.setText(availableCommands[currentCommandIndex]);
            loadTargetsForCommand(availableCommands[currentCommandIndex]);
        } else {
            commandText.setText("命令选择");
        }
    }
    
    private void loadTargetsForCommand(String command) {
        availableTargets = new String[0];
        currentTargetIndex = -1;
        targetContainer.removeAllViews();
        
        if (command.equals("go") || command.equals("talk")) {
            String targets = getCommandTargets(command);
            if (!TextUtils.isEmpty(targets)) {
                availableTargets = targets.split("\\|");
                currentTargetIndex = 0;
                updateTargetButtons();
            }
        }
    }
    
    private void updateTargetButtons() {
        targetContainer.removeAllViews();
        
        if (availableTargets.length > 0) {
            for (int i = 0; i < availableTargets.length; i++) {
                final int index = i;
                Button targetBtn = new Button(this);
                targetBtn.setText(availableTargets[i]);
                targetBtn.setPadding(16, 8, 16, 8);
                targetBtn.setBackgroundColor(0xff4a4a6a);
                targetBtn.setTextColor(0xffffffff);
                
                LinearLayout.LayoutParams params = new LinearLayout.LayoutParams(
                    LinearLayout.LayoutParams.WRAP_CONTENT,
                    LinearLayout.LayoutParams.WRAP_CONTENT);
                params.setMargins(4, 0, 4, 0);
                targetBtn.setLayoutParams(params);
                
                targetBtn.setOnClickListener(v -> {
                    selectTarget(index);
                });
                
                targetContainer.addView(targetBtn);
            }
        }
    }
    
    private void selectTarget(int index) {
        if (index < 0 || index >= availableTargets.length) return;
        currentTargetIndex = index;
        
        String cmd = availableCommands[currentCommandIndex];
        String target = availableTargets[currentTargetIndex];
        String fullCommand = cmd + " " + target;
        
        inputText.setText(fullCommand);
        sendMessage();
    }
    
    @Override
    public void onClick(View v) {
        int id = v.getId();
        if (id == R.id.sendBtn) {
            sendMessage();
        } else if (id == R.id.prevBtn) {
            switchTarget(-1);
        } else if (id == R.id.nextBtn) {
            switchTarget(1);
        }
    }
    
    private void switchTarget(int direction) {
        if (availableTargets.length > 1) {
            currentTargetIndex += direction;
            if (currentTargetIndex < 0) currentTargetIndex = availableTargets.length - 1;
            if (currentTargetIndex >= availableTargets.length) currentTargetIndex = 0;
            
            String cmd = availableCommands[currentCommandIndex];
            String target = availableTargets[currentTargetIndex];
            inputText.setText(cmd + " " + target);
        } else if (availableCommands.length > 1) {
            currentCommandIndex += direction;
            if (currentCommandIndex < 0) currentCommandIndex = availableCommands.length - 1;
            if (currentCommandIndex >= availableCommands.length) currentCommandIndex = 0;
            updateCommandText();
        }
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
            
            if (!availableCommands[0].equals("look")) {
                String[] temp = availableCommands.clone();
                availableCommands = new String[]{"look", "map", "inventory", "quest", "go", "talk"};
                currentCommandIndex = 0;
            }
            loadAvailableCommands();
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
    private native String getAvailableCommands();
    private native String getCommandTargets(String command);
}
