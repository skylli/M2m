import pym2m as p

print(dir(dir))

def m2mcallback(code, receive, args ):
    print "in m2m callback"
    print "python call back code: %d args is %s" % (code, args);
    return 0

def tstcallback( code, *args):
    print "in m2m callback %d" % code
    print(code)
    if(args.__len__() >= 2):
        print(args[0])
        print(args[1])
        return memoryview(args[1])
    else:
        print("args is :: ")
        print(args.__len__())
        print(args[0])
        return 0
    return 0


v = memoryview('abcefg')
print "start m2mInit"
p.m2mInit(1,0,1000,tstcallback,v)
nv = 'net callback'
lid = b'\x01'
hid = b'\x02\03'

net = p.netCreat( lid,9528, hid,"127.0.0.1", 9527,"111",tstcallback, nv )

while True:
    p.netTrysync(net)
