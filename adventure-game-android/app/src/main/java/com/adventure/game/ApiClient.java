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
            
            // 使用 chat 格式，优化参数平衡自然度和防重复
            String jsonBody = String.format(
                "{\"prompt\":\"USER: %s\\nASSISTANT:\",\"n_predict\":%d,\"temperature\":0.75,\"repeat_penalty\":1.15,\"top_k\":50,\"top_p\":0.92,\"stop\":[\"USER:\",\"ASSISTANT:\",\"\\n\\n\",\"玩家:\",\"NPC:\",\"【\"],\"stream\":false}",
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
            
            // 系统指令（强调第一人称和人设）
            prompt.append("【系统指令】\n");
            prompt.append("你正在扮演一个 RPG 游戏中的 NPC。请严格遵守以下规则：\n");
            prompt.append("1. 永远使用第一人称（我）回复，不要用'村长'、'铁匠'等第三人称\n");
            prompt.append("2. 像真人一样自然对话，不要重复对方的话\n");
            prompt.append("3. 回复要有 NPC 的个性和情感\n\n");
            
            // NPC 设定
            prompt.append("【你的角色】\n");
            prompt.append(npcContext);
            prompt.append("\n\n");
            
            // 对话历史
            if (conversationHistory != null && conversationHistory.length() > 0) {
                prompt.append("【之前的对话】\n");
                prompt.append(conversationHistory);
                prompt.append("\n");
            }
            
            // 玩家输入
            prompt.append("【玩家说】\n");
            prompt.append(playerSay);
            prompt.append("\n\n");
            
            // 回复格式示例（Few-shot）
            prompt.append("【正确示例】\n");
            prompt.append("玩家：你好\n");
            prompt.append("你：'你好，旅人。我是这个村的村长，有什么可以帮你的吗？'（微笑）\n\n");
            prompt.append("玩家：你是谁\n");
            prompt.append("你：'我叫老约翰，在这个村子住了 40 年啦。'\n\n");
            prompt.append("【错误示例】\n");
            prompt.append("❌ 村长对旅人表示欢迎\n");
            prompt.append("❌ 村长说道：你好\n");
            prompt.append("❌ 玩家说：你好 → 村长说：你好\n\n");
            
            // 回复要求
            prompt.append("【回复要求】\n");
            prompt.append("1. 用'我'而不是'村长'自称\n");
            prompt.append("2. 回复自然口语化（2-3 句话）\n");
            prompt.append("3. 可以加 (动作描写) 增强代入感\n");
            prompt.append("4. 不要复述玩家的话\n");
            prompt.append("5. 使用中文\n\n");
            
            prompt.append("【你的回复】\n");
            
            // 提高 temperature 增加自然度，但保留防重复
            return generateResponse(prompt.toString(), 150);
        } catch (Exception e) {
            return generateResponse(playerSay, 150);
        }
    }
}
