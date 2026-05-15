# Image Analysis Rule

## NEVER use Read for image files

If the current model is a non-Anthropic provider (GLM, Fireworks, OpenRouter, proxy) -- Read cannot process image content and will fail with `image_url is only supported by certain models` or return empty/unusable results.

Even if the model supports vision natively (Claude), prefer MCP-based analysis for consistency.

## Use youtube-tools MCP instead

For analyzing any image file (screenshots, video frames, photos, diagrams):

1. `mcp__youtube-tools__analyze_image_file` -- primary tool
   - `path`: local file path to the image
   - `prompt`: describe what to extract/analyze
   - MCP sends the file to a vision API independently and returns text

2. `mcp__youtube-tools__read_image_file` -- alternative for simple reads

## When this applies

- User asks to "analyze image", "look at screenshot", "read photo"
- Processing video frames or article images
- Any `.jpg`, `.jpeg`, `.png`, `.gif`, `.bmp`, `.webp` file that needs content analysis
- The Read tool is fine for non-image files (text, code, PDF)

## For video frame extraction

Use `mcp__youtube-tools__extract_video_frame` / `extract_video_frames` with `return_images=false` (save to disk), then analyze saved frames via `analyze_image_file`. Do NOT Read the saved frame files.
