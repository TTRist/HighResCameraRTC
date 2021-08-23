# HighResCameraRTC
高画質画像に対応してるカメラRTC．
exeがある階層にrtc.confをコピペする．

表示画像上でキーを押すと対応した動作をする．  
s...画像をセーブ．プロジェクトフォルダ直下にimg.pngを生成する．  
t...カメラの画像データをRTC::CameraImageポートで送信．  
f...オートフォーカスOFF．固定焦点  

## 固定フォーカスについて
バインドパラメータでRTSystemEditor上にて変更可能
