package com.adventure.game;

import android.Manifest;
import android.content.Intent;
import android.content.pm.PackageManager;
import android.net.Uri;
import android.os.Build;
import android.os.Bundle;
import android.os.Environment;
import android.provider.Settings;
import android.widget.Button;
import android.widget.TextView;
import android.widget.Toast;
import android.widget.EditText;
import android.view.View;

import androidx.activity.result.ActivityResultLauncher;
import androidx.activity.result.contract.ActivityResultContracts;
import androidx.annotation.NonNull;
import androidx.appcompat.app.AppCompatActivity;
import androidx.core.content.ContextCompat;

import java.io.File;
import java.io.FileOutputStream;
import java.io.InputStream;

public class MainActivity extends AppCompatActivity {
    
    private static final String[] PERMISSIONS = {
        Manifest.permission.READ_EXTERNAL_STORAGE,
        Manifest.permission.WRITE_EXTERNAL_STORAGE
    };
    
    private TextView statusText;
    private Button selectModelBtn;
    private Button startGameBtn;
    private TextView modelPathText;
    
    private String selectedModelPath = null;
    
    private final ActivityResultLauncher<String> filePickerLauncher = 
        registerForActivityResult(new ActivityResultContracts.GetContent(), result -> {
            if (result != null) {
                handleSelectedFile(result);
            }
        });
    
    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);
        
        initViews();
        checkPermissions();
    }
    
    private void initViews() {
        statusText = findViewById(R.id.statusText);
        selectModelBtn = findViewById(R.id.selectModelBtn);
        startGameBtn = findViewById(R.id.startGameBtn);
        modelPathText = findViewById(R.id.modelPathText);
        
        selectModelBtn.setOnClickListener(v -> openFilePicker());
        startGameBtn.setOnClickListener(v -> startGame());
        
        // 检查默认模型目录
        checkDefaultModelDir();
    }
    
    private void checkDefaultModelDir() {
        File modelsDir = new File(getExternalFilesDir(null), "models");
        if (!modelsDir.exists()) {
            modelsDir.mkdirs();
        }
        
        File[] modelFiles = modelsDir.listFiles((dir, name) -> name.endsWith(".gguf"));
        if (modelFiles != null && modelFiles.length > 0) {
            selectedModelPath = modelFiles[0].getAbsolutePath();
            modelPathText.setText("已找到：" + selectedModelPath);
            statusText.setText("检测到模型文件，可以直接开始游戏");
        } else {
            statusText.setText("请选择 GGUF 模型文件或放入 models 目录");
        }
    }
    
    private void checkPermissions() {
        if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.R) {
            if (!Environment.isExternalStorageManager()) {
                statusText.setText("需要文件管理权限来访问模型文件");
            }
        } else {
            boolean hasPermission = true;
            for (String permission : PERMISSIONS) {
                if (ContextCompat.checkSelfPermission(this, permission) 
                        != PackageManager.PERMISSION_GRANTED) {
                    hasPermission = false;
                    break;
                }
            }
            if (!hasPermission) {
                requestPermissions(PERMISSIONS, 100);
            }
        }
    }
    
    @Override
    public void onRequestPermissionsResult(int requestCode, @NonNull String[] permissions, 
                                           @NonNull int[] grantResults) {
        super.onRequestPermissionsResult(requestCode, permissions, grantResults);
        if (requestCode == 100) {
            boolean allGranted = true;
            for (int result : grantResults) {
                if (result != PackageManager.PERMISSION_GRANTED) {
                    allGranted = false;
                    break;
                }
            }
            if (allGranted) {
                statusText.setText("权限已授予");
            } else {
                statusText.setText("需要文件访问权限");
            }
        }
    }
    
    private void openFilePicker() {
        if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.R) {
            // Android 11+ 使用特殊权限
            if (!Environment.isExternalStorageManager()) {
                Intent intent = new Intent(Settings.ACTION_MANAGE_APP_ALL_FILES_ACCESS_PERMISSION);
                intent.setData(Uri.parse("package:" + getPackageName()));
                startActivity(intent);
            } else {
                filePickerLauncher.launch("*/*");
            }
        } else {
            filePickerLauncher.launch("*/*");
        }
    }
    
    private void handleSelectedFile(Uri uri) {
        try {
            // 复制文件到应用目录
            String fileName = getFileName(uri);
            if (!fileName.endsWith(".gguf")) {
                Toast.makeText(this, "请选择 GGUF 格式的模型文件", Toast.LENGTH_SHORT).show();
                return;
            }
            
            File destFile = new File(getExternalFilesDir(null), "models/" + fileName);
            destFile.getParentFile().mkdirs();
            
            InputStream input = getContentResolver().openInputStream(uri);
            FileOutputStream output = new FileOutputStream(destFile);
            
            byte[] buffer = new byte[8192];
            int bytesRead;
            while ((bytesRead = input.read(buffer)) != -1) {
                output.write(buffer, 0, bytesRead);
            }
            
            input.close();
            output.close();
            
            selectedModelPath = destFile.getAbsolutePath();
            modelPathText.setText("已选择：" + selectedModelPath);
            statusText.setText("模型已准备好，可以开始游戏");
            Toast.makeText(this, "模型文件已复制", Toast.LENGTH_SHORT).show();
            
        } catch (Exception e) {
            Toast.makeText(this, "选择文件失败：" + e.getMessage(), Toast.LENGTH_SHORT).show();
        }
    }
    
    private String getFileName(Uri uri) {
        String result = null;
        if (uri.getScheme() != null && uri.getScheme().equals("content")) {
            android.database.Cursor cursor = getContentResolver().query(uri, null, null, null, null);
            if (cursor != null) {
                try {
                    int nameIndex = cursor.getColumnIndex("_display_name");
                    if (nameIndex >= 0) {
                        cursor.moveToFirst();
                        result = cursor.getString(nameIndex);
                    }
                } finally {
                    cursor.close();
                }
            }
        }
        if (result == null) {
            result = uri.getLastPathSegment();
        }
        return result;
    }
    
    private void startGame() {
        Intent intent = new Intent(this, GameActivity.class);
        if (selectedModelPath != null) {
            intent.putExtra("model_path", selectedModelPath);
        }
        startActivity(intent);
    }
    
    @Override
    protected void onResume() {
        super.onResume();
        // 重新检查权限状态
        if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.R) {
            if (Environment.isExternalStorageManager()) {
                checkDefaultModelDir();
            }
        }
    }
}
