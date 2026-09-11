# 来源与许可

本目录由 `metasequoiaime/MSIME-Engine` 内嵌而来，来源与裁剪范围见 [UPSTREAM.md](UPSTREAM.md)。下述声明保留原样：仓库根 `LICENSE` 不覆盖也不重新授权其中的第三方代码与数据。

- 输入引擎本体：仓库根 `LICENSE`；`googlepinyinime-rev/` 与 `utfcpp/` 各自保留上游许可（见各自目录下的 `LICENSE`）。
- 辅助码：[helpcode/NOTICE.md](helpcode/NOTICE.md)。
- 语音：`voice/LICENSE`；`voice/third_party/miniaudio` 保留其上游许可。语音模型的来源、版本、许可与 SHA256 见 [voice/assets/models/README.md](voice/assets/models/README.md)：`silero_vad.onnx` 取自 snakers4/silero-vad v6.2（MIT）。
- 日本语模型发布时必须附带 `mozc_dictionary_oss_README.txt`。

词库数据不在本目录内。它由 `product-lock.json` 钉住的发布产物在构建时下载，其来源与许可随该发布一同分发；whisper.cpp 与本地推理权重已随专门化一并移除。
