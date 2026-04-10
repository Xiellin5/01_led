git合作方式：
1.我这边开一个主仓库（github，不是gitee），大家自己fork一下
2.大家写好代码后在github上填报pull request并提醒我审查和同意
3.需要更新代码库的话，可以在自己的库里pull原始仓库来更新



提交内容：
1.component文件夹
2.main文件夹
3.managed_components文件夹和dependencies.lock
4.Test文件夹
【注意：build不提交，可以手动在github desktop取消上传】



模块书写方式：components对应文件夹新建模块文件夹，一个模块单独一个CMakelists，最后再顶层对应位置加入，如果部分不需要使用，请先复制注释掉，再删掉对应行即可
