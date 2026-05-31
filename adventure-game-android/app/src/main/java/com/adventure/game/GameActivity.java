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
    
    // 快捷按钮
    private Button btnLook, btnMap, btnInventory, btnStatus;
    private Button btnQuest, btnTalk, btnMore;
    
    // 更多命令面板按钮
    private LinearLayout morePanel;
    private Button btnAppearance, btnMemory, btnNpc, btnGo;
    private Button btnGift, btnTrade, btnInteract, btnSetNpc;
    private Button btnCreateNpc, btnRemoveNpc;
    
    // 目标选择
    private LinearLayout targetContainer;
    
    private Handler handler = new Handler(Looper.getMainLooper());
    private boolean apiAvailable = false;
    
    private String[] availableTargets = {};
    private int currentTargetIndex = -1;
    private String currentCommand = "";
    private boolean morePanelVisible = false;
    
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
                    sb.append("\n游戏已加载！点击快捷按钮或输入命令\n");
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
        targetContainer = findViewById(R.id.targetContainer);
        
        // 快捷按钮
        btnLook = findViewById(R.id.btnLook);
        btnMap = findViewById(R.id.btnMap);
        btnInventory = findViewById(R.id.btnInventory);
        btnStatus = findViewById(R.id.btnStatus);
        btnQuest = findViewById(R.id.btnQuest);
        btnTalk = findViewById(R.id.btnTalk);
        btnMore = findViewById(R.id.btnMore);
        
        // 更多命令面板
        morePanel = findViewById(R.id.morePanel);
        btnAppearance = findViewById(R.id.btnAppearance);
        btnMemory = findViewById(R.id.btnMemory);
        btnNpc = findViewById(R.id.btnNpc);
        btnGo = findViewById(R.id.btnGo);
        btnGift = findViewById(R.id.btnGift);
        btnTrade = findViewById(R.id.btnTrade);
        btnInteract = findViewById(R.id.btnInteract);
        btnSetNpc = findViewById(R.id.btnSetNpc);
        btnCreateNpc = findViewById(R.id.btnCreateNpc);
        btnRemoveNpc = findViewById(R.id.btnRemoveNpc);
        btnGift = findViewById(R.id.btnGift);
        btnTrade = findViewById(R.id.btnTrade);
        btnInteract = findViewById(R.id.btnInteract);
        btnCreateNpc = findViewById(R.id.btnCreateNpc);
        btnRemoveNpc = findViewById(R.id.btnRemoveNpc);
        
        outputText.setMovementMethod(new ScrollingMovementMethod());
        
        // 绑定点击事件
        sendBtn.setOnClickListener(this);
        btnLook.setOnClickListener(this);
        btnMap.setOnClickListener(this);
        btnInventory.setOnClickListener(this);
        btnStatus.setOnClickListener(this);
        btnQuest.setOnClickListener(this);
        btnTalk.setOnClickListener(this);
        btnMore.setOnClickListener(this);
        btnAppearance.setOnClickListener(this);
        btnMemory.setOnClickListener(this);
        btnNpc.setOnClickListener(this);
        btnGo.setOnClickListener(this);
        btnGift.setOnClickListener(this);
        btnTrade.setOnClickListener(this);
        btnInteract.setOnClickListener(this);
        btnSetNpc.setOnClickListener(this);
        btnCreateNpc.setOnClickListener(this);
        btnRemoveNpc.setOnClickListener(this);
        
        inputText.setOnEditorActionListener((v, actionId, event) -> {
            sendMessage();
            return true;
        });
    }
    
    private void loadTargetsForCommand(String command) {
        availableTargets = new String[0];
        currentTargetIndex = -1;
        targetContainer.removeAllViews();
        
        // 需要目标的命令
        if (command.equals("go") || command.equals("talk") || 
            command.equals("gift") || command.equals("trade") || 
            command.equals("interact") || command.equals("npc") || 
            command.equals("remove_npc") || command.equals("setnpc")) {
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
        
        if (!currentCommand.isEmpty()) {
            String target = availableTargets[index];
            inputText.setText(currentCommand + " " + target);
        }
    }
    
    @Override
    public void onClick(View v) {
        int id = v.getId();
        
        if (id == R.id.sendBtn) {
            sendMessage();
        }
        else if (id == R.id.btnLook) {
            executeCommand("look");
        }
        else if (id == R.id.btnMap) {
            executeCommand("map");
        }
        else if (id == R.id.btnInventory) {
            executeCommand("inventory");
        }
        else if (id == R.id.btnStatus) {
            executeCommand("status");
        }
        else if (id == R.id.btnQuest) {
            executeCommand("quest");
        }
        else if (id == R.id.btnTalk) {
            currentCommand = "talk";
            loadTargetsForCommand("talk");
            inputText.setText("talk ");
            inputText.requestFocus();
        }
        else if (id == R.id.btnMore) {
            toggleMorePanel();
        }
        else if (id == R.id.btnAppearance) {
            executeCommand("appearance");
        }
        else if (id == R.id.btnMemory) {
            executeCommand("memory");
        }
        else if (id == R.id.btnNpc) {
            currentCommand = "npc";
            loadTargetsForCommand("npc");
            inputText.setText("npc ");
            inputText.requestFocus();
        }
        else if (id == R.id.btnGo) {
            currentCommand = "go";
            loadTargetsForCommand("go");
            inputText.setText("go ");
            inputText.requestFocus();
        }
        else if (id == R.id.btnGift) {
            currentCommand = "gift";
            loadTargetsForCommand("gift");
            inputText.setText("gift ");
            inputText.requestFocus();
        }
        else if (id == R.id.btnTrade) {
            currentCommand = "trade";
            loadTargetsForCommand("trade");
            inputText.setText("trade ");
            inputText.requestFocus();
        }
        else if (id == R.id.btnInteract) {
            currentCommand = "interact";
            loadTargetsForCommand("interact");
            inputText.setText("interact ");
            inputText.requestFocus();
        }
        else if (id == R.id.btnSetNpc) {
            currentCommand = "setnpc";
            loadTargetsForCommand("remove_npc"); // 使用自定义 NPC 列表
            inputText.setText("setnpc ");
            inputText.requestFocus();
        }
        else if (id == R.id.btnCreateNpc) {
            executeCommand("create_npc");
        }
        else if (id == R.id.btnRemoveNpc) {
            currentCommand = "remove_npc";
            loadTargetsForCommand("remove_npc");
            inputText.setText("remove_npc ");
            inputText.requestFocus();
        }
        else if (id == R.id.btnChat) {
            // AI 对话需要选择 NPC 后输入对话内容
            currentCommand = "chat";
            loadTargetsForCommand("talk"); // 使用当前场景 NPC 列表
            inputText.setText("chat ");
            inputText.requestFocus();
        }
        else if (id == R.id.btnAsk) {
            // AI 问答直接输入问题
            executeCommand("ask ");
        }
    }
    
    private void executeCommand(String cmd) {
        inputText.setText(cmd);
        sendMessage();
    }
    
    private void toggleMorePanel() {
        morePanelVisible = !morePanelVisible;
        morePanel.setVisibility(morePanelVisible ? View.VISIBLE : View.GONE);
        btnMore.setText(morePanelVisible ? "更多 ▲" : "更多 ▼");
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
        }
        else if (input.startsWith("chat ")) {
            // chat [NPC] [消息] 格式
            handleChatCommand(input.substring(5));
        }
        else if (input.startsWith("talk ")) {
            // 简单处理 talk 命令，使用固定回复（AI 对话用 chat）
            String response = processInput(input);
            onGameOutput(response);
            loadAvailableTargets();
        }
        else {
            String response = processInput(input);
            onGameOutput(response);
            loadAvailableTargets();
        }
        
        scrollView.post(() -> {
            scrollView.fullScroll(ScrollView.FOCUS_DOWN);
        });
    }
    
    // 处理 chat [NPC] [消息] 命令
    private void handleChatCommand(String args) {
        // 解析 NPC 名称和消息
        int spaceIdx = args.indexOf(' ');
        String npcName, message;
        
        if (spaceIdx > 0) {
            npcName = args.substring(0, spaceIdx);
            message = args.substring(spaceIdx + 1);
        } else {
            npcName = args;
            message = "你好";
        }
        
        handleNpcTalk(npcName, message);
    }
    
    private void loadAvailableTargets() {
        // 根据当前输入更新目标
        if (!currentCommand.isEmpty()) {
            loadTargetsForCommand(currentCommand);
        }
    }
    
    private void handleAskCommand(String question) {
        handleAskCommandInternal(question, null, null);
    }
    
    private void handleAskCommandInternal(String question, String npcName, String npcContext) {
        new Thread(() -> {
            try {
                String response;
                if (npcName != null && npcContext != null) {
                    // NPC 角色扮演对话
                    response = ApiClient.sendNpcRequest(npcContext, "", question);
                } else {
                    // 普通 AI 对话
                    response = ApiClient.sendRequest(question);
                }
                
                final String finalResponse = response;
                runOnUiThread(() -> {
                    if (finalResponse != null) {
                        onGameOutput("\n" + finalResponse);
                    } else {
                        onGameOutput("\nAI 服务器无响应");
                    }
                });
            } catch (Exception e) {
                runOnUiThread(() -> {
                    onGameOutput("\nAI 请求失败：" + e.getMessage());
                });
            }
        }).start();
    }
    
    // 处理 NPC 对话（带 AI）
    private void handleNpcTalk(String npcName, String playerSay) {
        new Thread(() -> {
            try {
                // 获取 NPC 上下文
                String npcContext = getNpcContext(npcName);
                if (npcContext == null || npcContext.isEmpty()) {
                    runOnUiThread(() -> onGameOutput("\n找不到 NPC：" + npcName));
                    return;
                }
                
                // 发送 AI 请求
                String response = ApiClient.sendNpcRequest(npcContext, "", playerSay);
                
                final String finalResponse = response;
                runOnUiThread(() -> {
                    if (finalResponse != null && !finalResponse.isEmpty()) {
                        onGameOutput("\n" + npcName + "：" + finalResponse);
                        // 保存对话记录
                        saveNpcTalk(npcName, playerSay, finalResponse);
                    } else {
                        onGameOutput("\n" + npcName + "：（沉默不语）");
                    }
                });
            } catch (Exception e) {
                runOnUiThread(() -> {
                    onGameOutput("\n" + npcName + "：（似乎没听清）");
                });
            }
        }).start();
    }
    
    private void onGameOutput(String text) {
        runOnUiThread(() -> {
            String current = outputText.getText().toString();
            outputText.setText(current + text + "\n");
        });
    }
    
    // JNI 函数声明
    public native boolean initGame(String modelPath);
    public native String processInput(String input);
    public native void cleanupGame();
    public native boolean isGameRunning();
    public native String getCurrentScene();
    public native int getGold();
    public native String getAvailableCommands();
    public native String getCommandTargets(String command);
    public native String getNpcContext(String npcName);
    public native void saveNpcTalk(String npcName, String playerSay, String npcReply);
}
