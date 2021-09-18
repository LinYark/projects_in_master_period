
# Run on the latest version of pytorch

# prepare training dataset

1.Put it in any folder,for me , I named one folder as dbpath。
The folder structure is：
```
    dbpath  /   dataset1
            /   dataset2
            /   ...
```

```
[got10k]链接: https://pan.baidu.com/s/1Zd1OnGZeH4koA3z6Usx3Bg  密码: tmjh
[LASOT]链接: https://pan.baidu.com/s/1BwsAmubB_itmREn3w2V-nQ  密码: qtd6
[LasHeR_fixed]链接: https://pan.baidu.com/s/1mgwc42LNFZm3bUFVdTGu1g  密码: f2vu
[rgbt234]链接: https://pan.baidu.com/s/1tFk3ewjwZHQhQ0lkN8KKfw  密码: sl8g
[GTOT]链接: https://pan.baidu.com/s/11xz-v2Q1EvJ-7vtp6ugd3Q  密码: we0u
```


2.Data standardization processing
Use the following python files to complete processing
Pay attention to modifying input and output in those files
```
.(siamdma)  /   data    /   prepareDataStanderd /
                                prepareDATAGOT10K.py
                                preparedataGTOT.py
                                ...
                                
python data/prepareDataStanderd/prepareDATAGOT10K.py
...

```




# prepare pretrain_models 
[pretrain_models]链接: https://pan.baidu.com/s/1Xqp38CrO5NZ-RSITONUyNg  密码: t7ar
put it in ./


# training
use ./bin/train.py to train our model
Pay attention to modifying the dbpath and jsonpath  

python bin/train.py

# eval
python bin/eval.py

# logger & lmdb & visdom 
They are already embedded in the framework
logger in ./util/funcs.py
lmdb in each prepareDataStanderd files
visdom in ./util/funcs.py
You can adjust the program to use them
