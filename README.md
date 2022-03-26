# optimizeAssociativeLearning
## 1. 実行環境
- 富岳
- NEURON 7.2 with MPI
- MPI environment
- Python2系
## 2. 実行準備
### 2.1 レポジトリのclone
- optimizeAssociativeLearningをclone
```
$ git clone git@github.com:Kenkentake/optimizeAssociativeLearning.git
```
- NeuronK+をclone
```
$ git clone git@github.com:Kenkentake/neuron_kplus.git
```
### 2.2 NeuronK+で実行ファイル（special）を作成
[このREADME](https://github.com/Kenkentake/neuron_kplus/blob/master/README.md)の「for K computer」を参考にして7.2のversionでspecialを作成
### 2.3設定ファイルの配置
- swcファイル
  - input/swc/alswcs/*.swc
  - input/swc/rnswcs/*.swc
- synlistファイル
  - input/synlist/alsynlist/*.txt
  - input/synlist/atorsynlist/*.txt
  - input/synlist/rtoasynlist/*.txt
  - input/synlist/rtoupsynlist/*.txt
### 2.4 パスの変更
パスの修正が必要なファイル
- src_min/job_estimate_main.sh
- src_min/run_interact_job.sh
## 3. 実行
### 3.1 コンパイル
```
$ cd src_min
$ ./run_compile.sh
```
### 3.2 シミュレーション+パラメータ推定の実行
- 通常ジョブ
```
$ pjsub job_estimate_main.sh
```
- 会話型ジョブ
```
$ ./enter_interact_job.sh
$ ./run_interact_job.sh
```

## 4 ジョブファイルのオプションについて
***- NUM_OF_POP***  
&emsp;CMA-ESの遺伝子数。「推奨値 = 4+3ln(入力次元数)」、大きければ大きい程良い。lambda % spawn_numprocs = 0になるように（子プロセス数で割り切れる数）設定。

***- MU***  
&emsp;CMA-ESでのエリート選択数。「推奨値 = lambda / 2」

***- NUM_OF_CHILD_PROCS***  
&emsp;生成する子プロセス数。基本は遺伝子数と同一にする。lambda % spawn_numprocs = 0になるように（子プロセス数で割り切れる数）設定。

***- MAXEVAL***  
&emsp;最大評価回数。イテレーション数と同義。

***- NUM_OF_GRANDCHILD_PROCS***  
&emsp;1孫プロセスあたりのニューロンプロセス数。

***- EXEC_PROG***  
&emsp;子世代がコールするMPI_Comm_spawnで実行するプログラムのパス

***- PARAMETER_FILENAME***  
&emsp;パラメータファイルのパス 


（DIM_CON_MATは使用しないため無視してください）

## 5 結果
### 5.1 結果ファイル
`src_min/al_result/{%m%d%H%M%S}`に結果フォルダが生成（jobfileでRESULT_DIR="./al_result/"と指定いる場合）
- cmaes/config.dat
  - 実行条件が記録
- cmaes/estimated_parameter.txt
  - 最終的な推定パラメータが記録
- cmaes/output_cmaes.dat
  - 評価値が記録（イテレーション数、イテレーション数*遺伝子数、過去の世代を含んだベスト適応度、その世代でのベスト適応度、その世代での適応度の平均）
- spike/{spike名}Spike.dat
  - 発火タイミングが記録
- voltage/{voltage名}.txt
  - 膜電位が記録（時間[t]、膜電位[mV]）
### 5.2 結果ファイルから発火率を算出
ganglionなどのpython2系が実行可能な環境で以下を実行。
標準出力として、30[ms]~691[ms]までの発火率[/s]が15[ms]間隔で表示。
```
$ cp src_min/{%m%d%H%M%S}/spikeuPN_mALT_ACh_DC3_57241Spike.dat visualizer
$ cd visualizer
$ python2 vis_spike_rate.py uPN_mALT_ACh_DC3_57241Spike.dat
```
