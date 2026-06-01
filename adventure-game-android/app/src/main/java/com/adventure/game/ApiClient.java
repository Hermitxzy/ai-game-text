package com.adventure.game;

import java.io.BufferedReader;
import java.io.InputStreamReader;
import java.io.OutputStream;
import java.net.HttpURLConnection;
import java.net.URL;
import java.nio.charset.StandardCharsets;

public class ApiClient {
    private static final int TIMEOUT_MS = 60000;
    
    public static String generateResponse(String prompt, int maxTokens) {
        try {
            URL url = new URL("http://127.0.0.1:8080/completion");
            HttpURLConnection conn = (HttpURLConnection) url.openConnection();
            conn.setConnectTimeout(TIMEOUT_MS);
            conn.setReadTimeout(TIMEOUT_MS);
            conn.setRequestMethod("POST");
            conn.setRequestProperty("Content-Type", "application/json");
            conn.setDoOutput(true);
            
            // 使用 chat 格式，避免 prompt 被原样返回
            String jsonBody = String.format(
                "{\"prompt\":\"USER: %s\\nASSISTANT:\",\"n_predict\":%d,\"temperature\":0.7,\"stop\":[\"USER:\",\"\\n\\n\"],\"stream\":false}",
                escapeJson(prompt),
                maxTokens
            );
            
            try (OutputStream os = conn.getOutputStream()) {
                byte[] input = jsonBody.getBytes(StandardCharsets.UTF_8);
                os.write(input, 0, input.length);
            }
            
            int status = conn.getResponseCode();
            BufferedReader br = new BufferedReader(
                new InputStreamReader(status < 300 ? conn.getInputStream() : conn.getErrorStream(), StandardCharsets.UTF_8)
            );
            
            StringBuilder response = new StringBuilder();
            String line;
            while ((line = br.readLine()) != null) {
                response.append(line);
            }
            br.close();
            
            if (status >= 200 && status < 300) {
                String parsed = parseResponse(response.toString());
                // 清理可能的前缀
                return parsed.replaceFirst("^\\s*", "");
            }
            return "API Error " + status + ": " + response.toString();
            
        } catch (Exception e) {
            return "Connection failed: " + e.getMessage();
        }
    }
    
    private static String parseResponse(String json) {
        try {
            // 尝试多种可能的字段名
            String[] fieldNames = {"content", "generation", "text", "response"};
            
            for (String fieldName : fieldNames) {
                String pattern = "\"" + fieldName + "\":\"";
                int start = json.indexOf(pattern);
                if (start >= 0) {
                    int valueStart = start + pattern.length();
                    int valueEnd = json.indexOf("\"", valueStart);
                    // 处理转义引号
                    while (valueEnd > valueStart && json.charAt(valueEnd - 1) == '\\') {
                        valueEnd = json.indexOf("\"", valueEnd + 1);
                    }
                    if (valueEnd > valueStart) {
                        String content = json.substring(valueStart, valueEnd);
                        return unescapeJson(content);
                    }
                }
            }
            
            // 如果没有找到字段，返回原始 JSON
            return json;
            
        } catch (Exception e) {
            return "解析错误：" + e.getMessage() + "\n原始响应：" + json;
        }
    }
    
    private static String escapeJson(String text) {
        return text.replace("\\", "\\\\")
                   .replace("\"", "\\\"")
                   .replace("\n", "\\n")
                   .replace("\r", "\\r")
                   .replace("\t", "\\t");
    }
    
    private static String unescapeJson(String text) {
        StringBuilder result = new StringBuilder();
        boolean escaped = false;
        
        for (int i = 0; i < text.length(); i++) {
            char c = text.charAt(i);
            
            if (escaped) {
                switch (c) {
                    case 'n': result.append('\n'); break;
                    case 'r': result.append('\r'); break;
                    case 't': result.append('\t'); break;
                    case '\"': result.append('\"'); break;
                    case '\\': result.append('\\'); break;
                    case 'u': 
                        // Unicode escape
                        if (i + 4 < text.length()) {
                            String hex = text.substring(i + 1, i + 5);
                            try {
                                int code = Integer.parseInt(hex, 16);
                                result.append((char) code);
                                i += 4;
                            } catch (NumberFormatException e) {
                                result.append("\\u").append(hex);
                            }
                        }
                        break;
                    default: result.append(c);
                }
                escaped = false;
            } else if (c == '\\') {
                escaped = true;
            } else {
                result.append(c);
            }
        }
        
        return result.toString();
    }
    
    public static boolean isServerRunning() {
        try {
            URL url = new URL("http://127.0.0.1:8080/health");
            HttpURLConnection conn = (HttpURLConnection) url.openConnection();
            conn.setConnectTimeout(3000);
            conn.setReadTimeout(3000);
            int status = conn.getResponseCode();
            conn.disconnect();
            return status >= 200 && status < 300;
        } catch (Exception e) {
            try {
                URL url = new URL("http://10.0.2.2:8080/health");
                HttpURLConnection conn = (HttpURLConnection) url.openConnection();
                conn.setConnectTimeout(3000);
                conn.setReadTimeout(3000);
                int status = conn.getResponseCode();
                conn.disconnect();
                return status >= 200 && status < 300;
            } catch (Exception e2) {
                return false;
            }
        }
    }
    
    public static String sendRequest(String prompt) {
        return generateResponse(prompt, 500);
    }
    
    // 带 NPC 上下文的角色扮演对话
    public static String sendNpcRequest(String npcContext, String conversationHistory, String playerSay) {
        try {
            StringBuilder prompt = new StringBuilder();
            
            // 系统指令（NPC 人设）
            prompt.append("你是一个角色扮演游戏中的 NPC。请完全沉浸在角色中，用第一人称回复。\n\n");
            prompt.append("【NPC 设定】\n");
            prompt.append(npcContext);
            prompt.append("\n\n");
            
            // 对话历史
            if (conversationHistory != null && conversationHistory.length() > 0) {
                prompt.append("【之前的对话记忆】\n");
                prompt.append(conversationHistory);
                prompt.append("\n");
            }
            
            // 玩家当前说的话
            prompt.append("【玩家当前输入】\n");
            prompt.append("玩家：");
            prompt.append(playerSay);
            prompt.append("\n\n");
            
            // 回复要求（更明确）
            prompt.append("【回复要求】\n");
            prompt.append("1. 只输出 NPC 的回复内容（对话 +简单动作描述）\n");
            prompt.append("2. 不要包含'用户说'、'玩家说'等字样\n");
            prompt.append("3. 保持 NPC 的人设和语气\n");
            prompt.append("4. 回复简洁（2-3 句话，50 字以内）\n");
            prompt.append("5. 使用中文回复\n");
            prompt.append("6. 不要提到你是 AI 或程序\n");
            prompt.append("7. 不要重复玩家的输入\n\n");
            
            prompt.append("【NPC 回复】\n");
            
            return generateResponse(prompt.toString(), 300);
        } catch (Exception e) {
            return generateResponse(playerSay, 300);
        }
    }
}
