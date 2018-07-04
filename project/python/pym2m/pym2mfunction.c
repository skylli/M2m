
// Python includes
#include <Python.h>

// STD includes
#include <stdio.h>
#include "../../../include/m2m_type.h"
#include "../../../include/m2m.h"
#include "../../../include/m2m_api.h"
#include "../../../include/util.h"
#include "../../../src/util/m2m_log.h"
#include "../../../src/network/network.h"


/* define ****************************************/
#define PY_EXC_ERR_FUNC(code, msg)	PyErr_SetString(code,msg)
#define PY_EXC_ERR_MEM(code, msg)	PY_EXC_ERR_FUNC(code, msg)

/* static variate**************************************************/
static M2M_conf_T py_m2mconf;

/* declara*********/
static char *py_encrypt_type[]={
	"no encode",
	"aes128",
	"broadcast",
	"none"
};
typedef struct PY_CALLBACK_T{
	PyObject *func;
	PyObject *args;
	void *p_args;
}Py_callback_T;
static M2M_conf_T m2m_conf;
//---- call back 
static PyObject *my_callback = NULL;

static PyObject *
my_set_callback(dummy, args)
    PyObject *dummy, *args;
{
    PyObject *result = NULL;
    PyObject *temp;

    if (PyArg_ParseTuple(args, "O:set_callback", &temp)) {
        if (!PyCallable_Check(temp)) {
            PyErr_SetString(PyExc_TypeError, "parameter must be callable");
            return NULL;
        }
        Py_XINCREF(temp);         /* Add a reference to new callback */
        Py_XDECREF(my_callback);  /* Dispose of previous callback */
        my_callback = temp;       /* Remember new callback */
        /* Boilerplate to return "None" */
        Py_INCREF(Py_None);
        result = Py_None;
    }
    return result;
}
static Py_callback_T *pycallback_creat(PyObject *func, PyObject *args){
	Py_callback_T  *p =  (Py_callback_T*)mmalloc(sizeof(Py_callback_T));
	_RETURN_EQUAL_0(p, 0);

	Py_XINCREF(func);
	Py_XINCREF(args);
	p->func = func;
	p->args = args;

	return p;
}
static void pycallback_destory(Py_callback_T *p_cb){
	if( !p_cb) return;
	
	Py_XDECREF(p_cb->func);
	Py_XDECREF(p_cb->args);
	mfree(p_cb);
}
#if 0
static int _obj_buffer_get( PyObject *py_obj, M2M_packet_T *p_pkt){
	// get ack buffer.
	if(py_obj && PyObject_CheckBuffer( py_obj )){
	
		Py_buffer *view = (Py_buffer *) malloc( sizeof(*view));
	
		if( PyObject_GetBuffer( py_obj, view, PyBUF_SIMPLE) ){
			// todo raise an error
			PY_EXC_ERR_FUNC(PyExc_BufferError, "m2m get ack buffer from py fail!");
			return -1;
		}
	
		if(view->buf && view->len ){
			p_pkt->p_data = mmalloc(view->len);
			if( !p_pkt->p_data ){ 
				//todo raise an error!				
				PY_EXC_ERR_FUNC(PyExc_BufferError, "memory alloc fail!");
				return -1;
			}
			//m2m_debug("get view buffer len %d", view->len);
			//m2m_bytes_dump("src buffer ", view->buf, view->len);
			p_pkt->len = view->len;
			mcpy( p_pkt->p_data,view->buf, view->len);						
		}		
		
		PyBuffer_Release(view);
		return p_pkt->len;
	}
	return 0;
}
#endif
static Py_callback_T *_obj_callback_get(PyObject *py_cb, PyObject *py_cb_arg, int must_flag){
	if (!PyCallable_Check(py_cb) && must_flag) {
			//do nothing ..
			PyErr_SetString(PyExc_TypeError, "There are no callback.");
            return NULL;
   	}
	
	return pycallback_creat(py_cb, py_cb_arg);
}
Py_buffer  _pybuffer;
#if 0
static PyObject *_obj_memoryview_build(u8 *p_buff, int len){

	PyObject *py_view = NULL;
	if( p_buff && len ){
		//Py_buffer pybuffer;
		if( PyBuffer_FillInfo(&_pybuffer, 0, p_buff, len, 0, PyBUF_CONTIG) == -1){
			//todo rais error
			PY_EXC_ERR_FUNC(PyExc_BufferError, "Can't conver to PyObject pybuffer !");
			return NULL;
		}			
		m2m_debug("py buffer len %d",len);
		m2m_bytes_dump("src buffer ", p_buff, len);
		
		py_view = PyMemoryView_FromBuffer(&_pybuffer);
		M2M_packet_T tst_pkt;
		_obj_buffer_get(py_view, &tst_pkt);
		mfree(tst_pkt.p_data);
		return py_view;
	}
	return NULL;
}
#endif
static void pym2m_callback(int code,M2M_packet_T **pp_ack_data,M2M_packet_T *p_recv_data,void *p_arg){

	m2m_log_debug("app_callback:: receive code = %d\n", code);
	// get python callback function 
	if( p_arg ){
		// get receive data
		PyObject *py_recv = NULL;
		if( p_recv_data && p_recv_data->len ){			
			py_recv = PyBytes_FromStringAndSize(p_recv_data->p_data, p_recv_data->len);
		}

		Py_callback_T *p_cb = (Py_callback_T*)p_arg;
		if(p_cb->func){
			PyObject *arglist = NULL;
			if(py_recv){
				m2m_bytes_dump("receive data", p_recv_data->p_data, p_recv_data->len);
				arglist = Py_BuildValue("(iOO)", code,p_cb->args, py_recv);
			}else
				arglist = Py_BuildValue("(iO)", code, p_cb->args);
			
			PyObject *py_ack = PyEval_CallObject(p_cb->func, arglist);

			//Py_DECREF(py_args);
			if(py_recv)
				Py_DECREF(py_recv);
			Py_DECREF(arglist);

			// get ack buffer.
			if(py_ack && PyObject_CheckBuffer( py_ack )){

				Py_buffer *view = (Py_buffer *) malloc( sizeof(*view));
				_RETURN_EQUAL_0(view, NULL);

				if( PyObject_GetBuffer( py_ack, view, PyBUF_SIMPLE) ){
					PyBuffer_Release(view);
					PY_EXC_ERR_FUNC(PyExc_BufferError, "callback alloc fail!");
					return ;
				}
				if(view->buf && view->len ){
					M2M_packet_T *p_ack = (M2M_packet_T*) mmalloc( sizeof(M2M_packet_T) );
					if( !p_ack ){ 
						//todo raise an error!
						PyBuffer_Release(view);
						PY_EXC_ERR_FUNC(PyExc_BufferError, "callback alloc fail!");
						return;
					}
					p_ack->p_data = mmalloc(view->len);
					if( !p_ack->p_data ){ 
						//todo raise an error!
						PyBuffer_Release(view);
						PY_EXC_ERR_FUNC(PyExc_BufferError, "callback alloc fail!");
						return;
					}

					mcpy( p_ack->p_data,view->buf, view->len);	
					p_ack->len = view->len;
					*pp_ack_data = p_ack;
				}				
				PyBuffer_Release(view);
			}			
		}
	}	
}
static void pym2m_callback_release(int code,M2M_packet_T **pp_ack_data,M2M_packet_T *p_recv_data,void *p_arg){

	pym2m_callback(code, pp_ack_data, p_recv_data, p_arg);
	if(p_arg){
		pycallback_destory( (Py_callback_T*)p_arg );
		p_arg = NULL;
	}
}

//-----------------------------------------------------------------------------
static PyObject *m2mInit(PyObject *self, PyObject *args)
{
	// Unpack a string from the arguments
	PyObject *cb_func, *cb_args;
	
	mmemset(&py_m2mconf, 0, sizeof(M2M_conf_T));
	
	if (!PyArg_ParseTuple(args, "iiiOO:Config_args", &py_m2mconf.def_enc_type, &py_m2mconf.do_relay, &py_m2mconf.max_router_tm, &cb_func, &cb_args)){
		PY_EXC_ERR_FUNC(PyExc_Exception, "get m2mInit args error !");		
		return NULL;
	}

	printf("enc: %d; relay: %d; max router time: %d, host_id: %s\n", py_m2mconf.def_enc_type, py_m2mconf.do_relay, py_m2mconf.max_router_tm );
	// get callback function 
	if (!PyCallable_Check(cb_func)) {
			//do nothing ..
			//PyErr_SetString(PyExc_TypeError, "There are no callback.");
            //return NULL;
   	}
	
#if 0
		// todo 
		PyObject  *arglist = NULL;
		M2M_packet_T recv;
		M2M_packet_T *p_recv_data = &recv;
		const char intsrt[4] = "012";
		recv.p_data = intsrt;
		recv.len = 4;
		PyObject *py_recv = NULL;
		if( p_recv_data && p_recv_data->len ){
			Py_buffer pybuffer;
			if( PyBuffer_FillInfo(&pybuffer, 0, p_recv_data->p_data, p_recv_data->len, 0, PyBUF_CONTIG) == -1){
				//todo rais error
				PY_EXC_ERR_FUNC(PyExc_BufferError, "m2m receive data can't conver to PyObject pybuffer !");
				return ;
			}			
			py_recv = PyMemoryView_FromBuffer(&pybuffer);
			arglist = Py_BuildValue("(iOO)", 200,cb_args,py_recv );
		}else {
			arglist = Py_BuildValue("(iO)", 200,cb_args );
		}
		
		PyObject *result = PyEval_CallObject(cb_func, arglist);
		
		Py_XDECREF(arglist);
		Py_XDECREF(result);
#endif

	Py_callback_T*py_cb = pycallback_creat(cb_func, cb_args);
	py_m2mconf.cb.func = pym2m_callback;
	py_m2mconf.cb.p_user_arg = (void*)py_cb;
	m2m_int(&py_m2mconf);


	// Print message and return None
	PySys_WriteStdout("m2m init successfully\n");
	Py_RETURN_NONE;
	
}
static PyObject *m2mDeinit(PyObject *self, PyObject *args){

	pycallback_destory( (Py_callback_T*)py_m2mconf.cb.p_user_arg);
	m2m_log_debug("m2m modeule have been destory");
}

//-----------------------------------------------------------------------------
static PyObject *netCreat(PyObject *self, PyObject *args){

	size_t net;
	int lport,hport, ret;
	const char *p_lid, *p_hid,*p_host;
	PyObject *py_cb, *py_cb_args;
	M2M_id_T lid,hid;
	char *p_key = NULL;
	Py_ssize_t  key_len =0, lid_len, hid_len;
	
	mmemset(&lid,0, sizeof(M2M_id_T));
	mmemset(&hid,0, sizeof(M2M_id_T));
	if( !PyArg_ParseTuple(args,"s#is#sis#OO:net creat arges", &p_lid, &lid_len, &lport, &p_hid, &hid_len, &p_host, &hport, &p_key, &key_len, &py_cb, &py_cb_args) ){
		PY_EXC_ERR_FUNC(PyExc_Exception, "get netCreat args error !");		
		return NULL;
	}

	// get local id 
	mcpy(lid.id,p_lid, M_MIN( (int)lid_len,ID_LEN) );
	// get host id
	if( hid_len > 0)
		mcpy(hid.id,p_hid, M_MIN( (int)hid_len,ID_LEN));

	m2m_bytes_dump("local id",lid.id, ID_LEN);
	m2m_printf("local host %d \n", lport);
	m2m_bytes_dump("host id",hid.id, ID_LEN);
	m2m_printf("host ip %s, port %d\n", p_host, hport);
	m2m_bytes_dump("local key ", p_key, key_len);
	// get call back 
	Py_callback_T *cb_arg = _obj_callback_get(py_cb, py_cb_args, 0);
	if(cb_arg)
		net = m2m_net_creat(&lid, lport, (int)key_len, p_key, &hid, p_host, hport, pym2m_callback, cb_arg);
	else
		net = m2m_net_creat(&lid, lport, (int)key_len, p_key, &hid, p_host, hport, NULL, NULL);

	m2m_debug("new net = %x ", net);
	//return _obj_memoryview_build((u8*)&net, sizeof(size_t));
	return PyBytes_FromStringAndSize((u8*)&net, sizeof(size_t));
}
static PyObject *netDestory(PyObject *self, PyObject *args){

	size_t net = 0;

	char *p_s_net = NULL;
	Net_T *p_net = NULL;
	M2M_packet_T net_pkt;
	PyObject *py_net;
	Py_ssize_t  net_len;
	mmemset(&net_pkt, 0, sizeof(M2M_packet_T));
	// get net  
	if(!PyArg_ParseTuple( args,"s#:net object", &p_s_net, &net_len) ){
		PY_EXC_ERR_FUNC( PyExc_Exception, "get netDestory args error !");		
		return NULL;
	}
	
	if( (int)net_len != (int)sizeof(size_t) ){
		PY_EXC_ERR_FUNC( PyExc_Exception, " netDestory can't find net !");		
		return NULL;
		}
	mcpy(&net, p_s_net, net_len);	
	p_net = net;	
	// destory  callback and arg
	if( p_net->callback.p_user_arg )
		pycallback_destory( (Py_callback_T*) p_net->callback.p_user_arg );
	// destory net
	p_net->callback.p_user_arg = NULL;
	m2m_net_destory(net);

	Py_RETURN_NONE;
}
static PyObject *netTrysync(PyObject *self, PyObject *args){

	size_t net = 0;

	char *p_s_net = NULL;
	Net_T *p_net = NULL;
	M2M_packet_T net_pkt;
	PyObject *py_net;
	Py_ssize_t  net_len;
	mmemset(&net_pkt, 0, sizeof(M2M_packet_T));
	// get net  
	if(!PyArg_ParseTuple( args,"s#:net object", &p_s_net, &net_len) ){
		PY_EXC_ERR_FUNC( PyExc_Exception, "get netTrysync args error !");		
		return NULL;
	}
	
	if( (int)net_len != (int)sizeof(size_t) ){
		PY_EXC_ERR_FUNC( PyExc_Exception, " netTrysync can't find net !");		
		return NULL;
		}
	mcpy(&net, p_s_net, net_len);	
	p_net = net;

	//m2m_debug("try sync net = %x pnet = %p", net,p_net);
	m2m_trysync( p_net );

	Py_RETURN_NONE;
}

//-----------------------------------------------------------------------------
static PyObject *sessionCreat(PyObject *self, PyObject *args){

	size_t net, session;
	int ret, rport;
	const char *py_net, *p_rid,*p_rhost, *p_rkey;
	PyObject *py_cb, *py_cb_args;
	M2M_id_T rid;
	char *p_key = NULL;
	Py_ssize_t  net_len =0, rid_len, rhost_len, rkey_len;
	
	mmemset(&rid,0, sizeof(M2M_id_T));
	if( !PyArg_ParseTuple(args,"s#s#s#s#iOO:creat session arges",\
		&py_net, &net_len, &p_rid, &rid_len, &p_rhost, &rhost_len, &p_rkey, &rkey_len, &rport, &py_cb, &py_cb_args)){
		PY_EXC_ERR_FUNC(PyExc_Exception, "get sessionCreat args error !");
		return NULL;
	}
	if( (int)net_len != (int)sizeof(size_t) ){
		PY_EXC_ERR_FUNC( PyExc_Exception, " sessionCreat can't find net !");		
		return NULL;
		}
	mcpy(&net, py_net, net_len);
	// get remote id 
	mcpy(rid.id,p_rid, M_MIN( (int)rid_len,ID_LEN) );
	
	Py_callback_T *cb_arg = _obj_callback_get(py_cb, py_cb_args, 0);
	session = m2m_session_creat(net, &rid, p_rhost, rport, (int)rkey_len, p_rkey,pym2m_callback, (void*)cb_arg);

	if( !session){
		pycallback_destory(cb_arg);
		PY_EXC_ERR_FUNC( PyExc_Exception, "session creat failt !");
		return NULL;
	}

	m2m_debug("new session = %x ", session);
	return PyBytes_FromStringAndSize((u8*)&session, sizeof(size_t));
}
static PyObject *sessionDestory(PyObject *self, PyObject *args){
	 M2M_T m2m;
	 
}
static PyObject *sessionTokenUpdate(PyObject *self, PyObject *args){}
static PyObject *sessionSecretkeySet(PyObject *self, PyObject *args){}
static PyObject *netBroadcastStart(PyObject *self, PyObject *args){}
static PyObject *netBroadcastStop(PyObject *self, PyObject *args){}
static PyObject *netBroadcastEnable(PyObject *self, PyObject *args){}
static PyObject *netBroadcastDisable(PyObject *self, PyObject *args){}
static PyObject *sessionSendData(PyObject *self, PyObject *args){}
static PyObject *netOnlineCheck(PyObject *self, PyObject *args){}



/* regist api */
PyMethodDef pym2m_methods[] = {
  {
    "m2mInit",
    m2mInit,
    METH_VARARGS,
    "config m2m modle \t usage: .m2mInit( encode_tyepe, is_relay, max router time, modle callback function, callback args)\n "
    "\t encode_tyepe: 1-aes128,2-broadcast;\t is_relay: 1-relay package,0-no relay package.\n"
  },  
  {
    "m2mDeinit",
    m2mDeinit,
    METH_VARARGS,
    "destory mem \t usage: .m2mDeinit\n"
  },
   {
    "netCreat",
    netCreat,
    METH_VARARGS,
    "creat an socket to receive an send data.\t"
    "usage: .netCreat( local id, local port,  server id, server host, server's port, secret key, callback function, callback args)\n "
  },
  {
    "netDestory",
    netDestory,
    METH_VARARGS,
    "destory mem \t usage: .netDestory( net).\t"
    "net: MemoryView objects that hold net Py_buffer.\n"
  },
    {
    "netTrysync",
    netTrysync,
    METH_VARARGS,
    "trysync  \t usage: .netTrysync( net).\t"
    "net: MemoryView objects that hold net Py_buffer.\n"
  },
  /* 
  {
    "elevation",
    elevation_example,
    METH_VARARGS,
    "Returns elevation of Nevado Sajama."
  }, */
  {NULL, NULL, 0, NULL}        /* Sentinel */
};

