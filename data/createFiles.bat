FOR /L %%A IN (1,1,100) DO (fsutil file createnew A%%A.x 1000)
FOR /L %%A IN (1,1,100) DO (fsutil file createnew B%%A.x 10000)
FOR /L %%A IN (1,1,100) DO (fsutil file createnew C%%A.x 100000)
FOR /L %%A IN (1,1,100) DO (fsutil file createnew D%%A.x 1000000)
fsutil file createnew E.x 10000000
fsutil file createnew F.x 100000000
fsutil file createnew G.x 1000000000