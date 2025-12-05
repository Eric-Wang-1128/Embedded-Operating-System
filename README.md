# 1132-EOS Lab&HW
## Hw1｜外送管理系統（單機版，RPi）
- 單人點餐流程的模擬，含 3 家店、各 2 道餐點，操作包含 shop list、order（含 confirm / cancel）。
- 功能重點：CLI 互動式選單、狀態管理（選店、選品、數量、確認／取消）、以 C+Makefile 結構化程式專案。

## Hw2｜外送平台（多人連線版：單一外送員、一次一位客戶）
- 把 Hw1 改成 socket server 版；同時間只服務一位客戶，支援 shop list、order 等協定互動（助教提供 client 檢測）。
- 功能重點：TCP socket 程式設計（Server 端）、封包格式與指令解析、阻塞式 I/O、正確回覆訊息（結尾換行等）。

## Hw3｜外送平台（多人連線版：兩位外送員、可同時多客戶）
- 採 process 或 thread 改成多客戶Concurrent，兩位外送員、FCFS 排隊，須避免 race condition；沿用 Hw2 指令並新增長等待的互動。
- 功能重點：多程序／多執行緒Concurrent、同步與共享狀態一致性、排程（FCFS）。

## Lab 3-1｜學號跑馬燈（RPi）
- 撰寫 kernel driver + user-space writer，在硬體上做學號的跑馬燈顯示（可控制方向與速度）。
- 功能重點：Linux kernel module、/dev 字元裝置、基本 I/O 控制。

## Lab 3-2｜學號跑馬燈－七段顯示器（RPi）
- 以 GPIO 驅動 7-seg，實作 driver + writer 呈現學號跑馬燈。
- 功能重點：GPIO 腳位控制、七段碼（segment/位掃）、ISR/時間節拍與顯示時序。

## Lab 4｜簡易名字跑馬燈（VM）
- 在 VM 的終端機以 ASCII/16-seg 方式捲動顯示名字，只需 user-space 程式。
- 功能重點：字型切片／映射、字串緩衝與位移動畫、輸出排版。

## Lab 5｜東方快車
- Server/Client 範例，伺服器以定時／訊號機制推動列車動畫，並提供 demo.sh 檢查與展示。
- 功能重點：Socket programming、signals 與 timer、檔案描述元操作（如 dup2）、子行程管理／避免Zombie process。

## Lab 6｜Web ATM
- ATM 伺服器，支援多連線存提款／查詢等操作，需處理競態、維持一致性，提供 client 命令格式。
- 功能重點：Concurrent處理（多客戶）、臨界區／一致性控制、通訊協定與錯誤處理。

## Lab 7｜終極密碼（猜數字對戰）
- 兩支程式（game / guess）透過 shared memory 與 signal/timer 互動，根據提示範圍縮小直到猜中。
- 功能重點：IPC（Shared Memory、FIFO）、訊號與計時器、程序間同步與資源清理。

## Final Project: 快速出單分配任務伺服器

專案概述：
本專案的目標是實作一個快速出單系統，模擬外送管理流程，並涵蓋點單系統、廚房系統及顯示介面。系統主要由三大部分構成：
- 點單系統：包含訂單建立、確認及取消功能，並支援與廚房系統的資料交互。

- 廚房系統：負責處理訂單並將任務分配給廚師，確保每道菜品按照正確的順序完成。

- 顯示介面：透過 GUI 顯示當前訂單狀態及進度，並與硬體（如七段顯示器）進行互動。

技術架構：
- 同步處理：使用 semaphore 控制資料可用性，並利用 mutex 保護資料一致性。

- 多執行緒處理：使用多執行緒分配任務給每個廚師，並確保多任務處理時的競爭條件（race condition）得到有效避免。

- 動態記憶體管理：每筆訂單由 kmalloc() 建立，並由 kfree() 回收，以確保內存管理的效率與安全。

- GPIO 驅動：透過 GPIO 控制外接七段顯示器，顯示當前訂單的編號。

核心功能：
- CLI 互動式選單：用戶可以選擇餐廳、菜品及數量，並確認或取消訂單。

- TCP Socket 通訊：系統使用 TCP Socket 實現 Server-Client 溝通，支持多客戶端的交互。

- 訂單管理與分配：廚房系統依照訂單優先順序進行任務分配，並同步處理每位廚師的工作進度。

- GUI 顯示：透過 GUI 顯示訂單狀態，並將當前完成的訂單編號顯示在七段顯示器上。