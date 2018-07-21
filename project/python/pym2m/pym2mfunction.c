
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

#define PY_EXC_MSG_RETURN(c,msg,r)		do{ PY_EXC_ERR_FUNC(c,msg); return r; }while(0)
#define PY_RETURN_EQUAL(n,c, err,msg,r)	do{ if(n == c){PY_EXC_MSG_RETURN(c,msg,r);}}while(0)
#define PY_RETURN_UNEQUAL(n,c, err,msg,r)	do{ if(n != c){PY_EXC_MSG_RETURN(c,msg,r);}}while(0)

#define PY_RETURN_EQUSL_0(n,err,msg,r)	PY_RETURN_EQUAL(n, 0, err,msg,r)

#define PY_RETURN_UNEQUSL_0(n,err,msg,r)	PY_RETURN_UNEQUAL(n, 0, err,msg,r)

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
static PyObject *receive_tuple_creat( int code,void *p_recv, PyObject *py_args){
	PyObject *py_recv = NULL, *py_rtupe = NULL;
	M2M_packet_T *p_recv_data = NULL;
	M2M_obs_payload_T *p_robs = NULL;
	
	if( !p_recv){
		return Py_BuildValue("(iO)", code,py_args);
	};

	switch(code){
		case M2M_REQUEST_OBSERVER_RQ:
		case M2M_REQUEST_NOTIFY_ACK:
		case M2M_ERR_OBSERVER_DISCARD:
			p_robs = (M2M_obs_payload_T*) p_recv;
			if(p_robs->p_obs_node ){
				size_t notifyIndex = p_robs->p_obs_node;
				m2m_log_debug("notify index %p", p_robs->p_obs_node );
				PyObject *py_notifyIndex = PyBytes_FromStringAndSize((const char*)&notifyIndex, sizeof(size_t));
				if(p_robs->p_payload && p_robs->p_payload->p_data && p_robs->p_payload->len > 0){
					m2m_log_debug("###>>>> payload len %d, payload: %s",p_robs->p_payload->len ,p_robs->p_payload->p_data );
					py_recv = PyBytes_FromStringAndSize(p_robs->p_payload->p_data, p_robs->p_payload->len);
					py_rtupe = Py_BuildValue("(iOOO)", code,py_args, py_notifyIndex,py_recv);
					//py_rtupe = Py_BuildValue("(iOiO)", code,py_notifyIndex, 1234,py_args);
				} 
				else py_rtupe = Py_BuildValue("(iOO)", code,py_args, py_notifyIndex);
			}
			else py_rtupe = Py_BuildValue("(iO)", code,py_args);
			break;
		default:
			p_recv_data = (M2M_packet_T*) p_recv;
			if( p_recv_data && p_recv_data->p_data && p_recv_data->len > 0){
					py_recv = PyBytes_FromStringAndSize( p_recv_data->p_data, p_recv_data->len);
					py_rtupe = Py_BuildValue("(iOO)", code,py_args,py_recv);
			} 
			else py_rtupe = Py_BuildValue("(iO)", code,py_args);
			break;
	}
	return py_rtupe;
}
static void pym2m_callback(int code,M2M_packet_T **pp_ack_data,void *p_r,void *p_arg){

	m2m_log_debug("app_callback:: receive code = %d\n", code);
	// get python callback function 
	if( p_arg ){
		// get receive data
		Py_callback_T *p_cb = (Py_callback_T*)p_arg;
		if(p_cb->func){
			PyObject *arglist = receive_tuple_creat(code, p_r, (PyObject*)p_cb->args);
			if( arglist == NULL){
				PY_EXC_ERR_FUNC(PyExc_Exception, "call back args Py_BuildValue failt!!");
				return;
			}
			PyObject *py_ack = PyObject_CallObject(p_cb->func, arglist);
			Py_DECREF(arglist);
			// destory arg
			if( code == M2M_ERR_REQUEST_DESTORY && p_cb){
				pycallback_destory(p_cb);
				p_cb = NULL;
				}
			if( py_ack == NULL ){
				//PY_EXC_ERR_FUNC(PyExc_Exception, "callback fail!");
				return ;
			}
			
			// get ack buffer.
			if( PyObject_CheckBuffer( py_ack )){
				Py_buffer *view = (Py_buffer *) malloc( sizeof(*view));
				_RETURN_EQUAL_0(view, NULL);

				if( PyObject_GetBuffer( py_ack, view, PyBUF_SIMPLE) ){
					Py_XDECREF(py_ack);
					PyBuffer_Release(view);
					PY_EXC_ERR_FUNC(PyExc_BufferError, "callback alloc fail!");
					return ;
				}
				// free py ack
				Py_XDECREF(py_ack);
				if(view->buf && view->len && pp_ack_data ){
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
						mfree( p_ack);
						PyBuffer_Release(view);
						PY_EXC_ERR_FUNC(PyExc_BufferError, "callback alloc fail!");
						return;
					}

					mcpy( p_ack->p_data,view->buf, view->len);	
					p_ack->len = view->len;
					*pp_ack_data = p_ack;
				}				
				PyBuffer_Release(view);
			}else Py_XDECREF(py_ack);
				
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
	PyObject *cb_func = NULL, *cb_args = NULL;
	
	mmemset(&py_m2mconf, 0, sizeof(M2M_conf_T));
	printf("in m2minit \n");
	if (!PyArg_ParseTuple(args, "i|OOii:(enc type, callback function, callback args)",\
		&py_m2mconf.def_enc_type, &cb_func, &cb_args,&py_m2mconf.do_relay, &py_m2mconf.max_router_tm)){
		PY_EXC_ERR_FUNC(PyExc_Exception, "get m2mInit args error !");		
		return NULL;
	}

	printf("enc: %d; relay: %d; max router time: %d\n", py_m2mconf.def_enc_type, py_m2mconf.do_relay, py_m2mconf.max_router_tm );
#if 1
	// get callback function 
	if(cb_func){
		printf(" get callback function.\n");
		if (!PyCallable_Check(cb_func)) {
			//do nothing ..
			PY_EXC_ERR_FUNC(PyExc_TypeError, "no callback functio found.");
            return NULL;
   		}
		Py_callback_T*py_cb = pycallback_creat(cb_func, cb_args);
		py_m2mconf.cb.func = pym2m_callback;
		py_m2mconf.cb.p_user_arg = (void*)py_cb;
	}
	m2m_int(&py_m2mconf);
#endif

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

	size_t net = 0;
	int lport = 0,hport = 0, ret = 0;
	const char *p_lid = NULL, *p_hid= NULL,*p_host = NULL;
	PyObject *py_cb = NULL, *py_cb_args = NULL;
	M2M_id_T lid,hid;
	char *p_key = NULL;
	Py_ssize_t  key_len =0, lid_len = 0, hid_len = 0;
	
	mmemset(&lid,0, sizeof(M2M_id_T));
	mmemset(&hid,0, sizeof(M2M_id_T));
	
	if( !PyArg_ParseTuple(args,"s#is#|OOs#si:(locak id, locak port, locak key,callback function,callback args,sever id, server host, server port)",\
		&p_lid, &lid_len, &lport,&p_key, &key_len, &py_cb, &py_cb_args,&p_hid, &hid_len, &p_host, &hport) ){
		PY_EXC_ERR_FUNC(PyExc_Exception, "get netCreat args error !");		
		return NULL;
	}

	// get local id 
	mcpy( lid.id,p_lid, M_MIN( (int)lid_len,ID_LEN));
	// get host id
	if( hid_len > 0)
		mcpy(hid.id,p_hid, M_MIN( (int)hid_len,ID_LEN));

	m2m_bytes_dump("local id",lid.id, ID_LEN);
	m2m_printf("local host %d \n", lport);
	m2m_bytes_dump("host id",hid.id, ID_LEN);
	m2m_printf("host ip %s, port %d\n", p_host, hport);
	m2m_bytes_dump("local key ", p_key, key_len);
	// get call back 
	if(py_cb){			
		Py_callback_T *cb_arg = _obj_callback_get(py_cb, py_cb_args, 0);
		if(cb_arg)
			net = m2m_net_creat(&lid, lport, (int)key_len, p_key, &hid, p_host, hport, pym2m_callback, cb_arg);

	}else 
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
	if(!PyArg_ParseTuple( args,"s#:(net)", &p_s_net, &net_len) ){
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
	Py_ssize_t  net_len;
	// get net  
	if(!PyArg_ParseTuple( args,"s#:(net)", &p_s_net, &net_len) ){
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
	if( !PyArg_ParseTuple(args,"s#s#s#s#iOO:(net,remote id, remote host, remote key, remote port, callback function, callback args)",\
		&py_net, &net_len, &p_rid, &rid_len, &p_rhost, &rhost_len, &p_rkey, &rkey_len, &rport, &py_cb, &py_cb_args)){
		PY_EXC_MSG_RETURN( PyExc_Exception, "get sessionCreat args error", NULL);		
	}
	if( (int)net_len != (int)sizeof(size_t) ){
		PY_EXC_MSG_RETURN( PyExc_Exception, " sessionCreat can't find net !", NULL);		
		}
	mcpy(&net, py_net, net_len);
	// get remote id 
	mcpy(rid.id,p_rid, M_MIN( (int)rid_len,ID_LEN));
	
	Py_callback_T *cb_arg = _obj_callback_get(py_cb, py_cb_args, 0);
	if(cb_arg == NULL)	
		return NULL;

	session = m2m_session_creat(net, &rid, p_rhost, rport, (int)rkey_len, p_rkey,pym2m_callback, (void*)cb_arg);
	if( !session){
		pycallback_destory(cb_arg);
		PY_EXC_MSG_RETURN(PyExc_Exception, "session creat failt", NULL);
	}

	m2m_debug("new session = %x ", session);
	return PyBytes_FromStringAndSize((u8*)&session, sizeof(size_t));
}
static PyObject *sessionDestory(PyObject *self, PyObject *args){
	M2M_T m2m;
	
	char *p_s_net = NULL, *p_s_s = NULL;
	M2M_packet_T net_pkt;
	PyObject *py_net;
	Py_ssize_t  net_len, s_len;
	mmemset(&net_pkt, 0, sizeof(M2M_packet_T));
	// get net  
	if(!PyArg_ParseTuple( args,"s#s#:(net,session)", &p_s_net, &net_len, &p_s_s, &s_len) ){
		PY_EXC_MSG_RETURN(PyExc_Exception, "get netTrysync args error !", NULL);
	}
	PY_RETURN_UNEQUAL((int)net_len, (int)sizeof(size_t), PyExc_Exception, " sessionDestory can't find net !", NULL);
	mcpy(&m2m.net , p_s_net, net_len);	

	PY_RETURN_UNEQUAL((int)s_len, (int)sizeof(size_t), PyExc_Exception, " sessionDestory can't find session !", NULL);
	mcpy(&m2m.session, p_s_s, s_len);
	
	m2m_session_destory(&m2m);
	Py_RETURN_NONE; 
}
static PyObject *sessionSendData(PyObject *self, PyObject *args){

	int ret = 0;
	M2M_T m2m;
	char *p_s_net = NULL, *p_s_s = NULL, *p_data = NULL;
	PyObject *py_cb = NULL, *py_cb_args = NULL;
	Py_ssize_t  net_len = 0, s_len = 0, data_len = 0;
	Py_callback_T *cb_arg = NULL;

	// get net  
	if(!PyArg_ParseTuple( args,"s#s#s#|OO:(net,session,data, callback function, callback args)",\
		&p_s_net, &net_len, &p_s_s, &s_len, &p_data, &data_len, &py_cb, &py_cb_args) ){
		PY_EXC_MSG_RETURN(PyExc_Exception, "get netTrysync args error !", NULL);
	}
	PY_RETURN_UNEQUAL((int)net_len, (int)sizeof(size_t), PyExc_Exception, " sessionSendData can't find net !", NULL);
	mcpy(&m2m.net , p_s_net, net_len);	

	PY_RETURN_UNEQUAL((int)s_len, (int)sizeof(size_t), PyExc_Exception, " sessionSendData can't find session !", NULL);
	mcpy(&m2m.session, p_s_s, s_len);

	if(py_cb){
		cb_arg = _obj_callback_get(py_cb, py_cb_args, 0);
		if(cb_arg == NULL)	
			return NULL;
	}

	ret = m2m_session_data_send(&m2m,(int)data_len, (u8*)p_data, pym2m_callback,(void*)cb_arg);

	return Py_BuildValue("i",ret);
}
static PyObject *sessionTokenUpdate(PyObject *self, PyObject *args){

	int ret = 0;
	M2M_T m2m;
	char *p_s_net = NULL;
	Py_ssize_t	net_len = 0, s_len = 0;
	PyObject *py_cb = NULL, *py_cb_args = NULL;
	Py_callback_T *cb_arg = NULL;
	// get net	
	if(!PyArg_ParseTuple( args,"s#s#|OO:(net,session, callback function, callback args)", \
		&m2m.net, &net_len, &m2m.session, &s_len, &py_cb, &py_cb_args) ){
		PY_EXC_ERR_FUNC( PyExc_Exception, "get sessionTokenUpdate args error !");		
		return NULL;
	}

	if( (int)net_len != (int)sizeof(size_t) ){
		PY_EXC_MSG_RETURN( PyExc_Exception, " session update token can't find net !", NULL);		
		}
	// get call back function and args.
	if(py_cb){
		cb_arg = _obj_callback_get(py_cb, py_cb_args, 0);
		if(cb_arg == NULL)	
			return NULL;
	}
	
	ret = m2m_session_token_update(&m2m, pym2m_callback, (void*)cb_arg);
	
	return Py_BuildValue("i",ret);
}
static PyObject *sessionSecretkeySet(PyObject *self, PyObject *args){

	int ret = 0;
	M2M_T m2m;
	char *p_s_net = NULL, *p_s_s = NULL, *p_data = NULL;
	PyObject *py_cb = NULL, *py_cb_args = NULL;
	Py_ssize_t  net_len = 0, s_len = 0, data_len = 0;
	Py_callback_T *cb_arg = NULL;

	// get net  
	if(!PyArg_ParseTuple( args,"s#s#s#|OO:(net, sessionm,new sesssion key,callback function, callback args).", &p_s_net, &net_len, &p_s_s, &s_len, &p_data, &data_len, &py_cb, &py_cb_args) ){
		PY_EXC_MSG_RETURN(PyExc_Exception, "get sessionSecretkeySet args error !", NULL);
	}
	PY_RETURN_UNEQUAL((int)net_len, (int)sizeof(size_t), PyExc_Exception, "update session secret key can't find net !", NULL);
	mcpy(&m2m.net , p_s_net, net_len);	

	PY_RETURN_UNEQUAL((int)s_len, (int)sizeof(size_t), PyExc_Exception, "update session secret key can't find session !", NULL);
	mcpy(&m2m.session, p_s_s, s_len);
	
	if(py_cb){
		cb_arg = _obj_callback_get(py_cb, py_cb_args, 0);
			if(cb_arg == NULL)	
				return NULL;
	}
	
	ret = m2m_session_secret_set(&m2m,(int)data_len, (u8*)p_data, pym2m_callback,(void*)cb_arg);
	return Py_BuildValue("i",ret);
}
static PyObject *netSecretkeySet(PyObject *self, PyObject *args){

	size_t net = 0;
	int ret, rport = 0;
	const char *py_net = NULL, *p_rid = NULL,*p_rhost = NULL, *p_rkey = NULL, *p_nkey;
	PyObject *py_cb, *py_cb_args = NULL;
	M2M_id_T rid;
	Py_ssize_t  net_len =0, rid_len = 0, rhost_len = 0, rkey_len = 0, nkey_len;
	Py_callback_T *cb_arg = NULL;

	mmemset(&rid,0, sizeof(M2M_id_T));
	if( !PyArg_ParseTuple(args,"s#s#s#is#s#|OO:(net, remote id, remote host, remote port,old remote key, new remote keym, callback function, callback args)",\
		&py_net, &net_len, &p_rid, &rid_len, &p_rhost, &rhost_len, &rport, &p_rkey, &rkey_len, &p_nkey, &nkey_len,&py_cb, &py_cb_args)){
		PY_EXC_MSG_RETURN( PyExc_Exception, "Get netSecretKetSet args error ", NULL);		
	}
	if( (int)net_len != (int)sizeof(size_t) ){
		PY_EXC_MSG_RETURN( PyExc_Exception, "netSecretKetSet can't find net !", NULL);		
		}
	mcpy(&net, py_net, net_len);
	// get remote id 
	mcpy(rid.id,p_rid, M_MIN( (int)rid_len,ID_LEN));

	if(py_cb){
		cb_arg = _obj_callback_get(py_cb, py_cb_args, 0);
		if(cb_arg == NULL)	
			return NULL;
	}
	ret = m2m_net_secretkey_set(net, &rid, p_rhost, rport, (int)rkey_len, p_rkey, nkey_len, p_nkey, pym2m_callback, (void*)cb_arg);
	return Py_BuildValue("i",ret);
}
static PyObject *sessionObserverStart(PyObject *self, PyObject *args){

	int ret = 0, ackType = 0;
	M2M_T m2m;
	char *p_s_net = NULL, *p_s_s = NULL, *p_data = NULL;
	PyObject *py_cb = NULL, *py_cb_args = NULL;
	Py_ssize_t  net_len = 0, s_len = 0, data_len = 0;
	Py_callback_T *cb_arg = NULL;
	
	// get net  
	if(!PyArg_ParseTuple( args,"s#s#is#|OO:(net, session, ack-type, data,callback function, callback args)", &p_s_net, &net_len, &p_s_s, &s_len, &ackType, &p_data, &data_len, &py_cb, &py_cb_args) ){
		PY_EXC_MSG_RETURN(PyExc_Exception, "get sessionObserverStart args error !", NULL);
	}
	PY_RETURN_UNEQUAL((int)net_len, (int)sizeof(size_t), PyExc_Exception, "sessionObserverStart can't find net !", NULL);
	mcpy(&m2m.net , p_s_net, net_len);	

	PY_RETURN_UNEQUAL((int)s_len, (int)sizeof(size_t), PyExc_Exception, "sessionObserverStart can't find session !", NULL);
	mcpy(&m2m.session, p_s_s, s_len);
	
	if(py_cb){
		cb_arg = _obj_callback_get(py_cb, py_cb_args, 0);
		if(cb_arg == NULL)	
			return NULL;
	}
	
	ret = m2m_session_observer_start(&m2m, ackType, (int)data_len, (u8*)p_data, pym2m_callback,(void*)cb_arg);
	return Py_BuildValue("i",ret);
}

static PyObject *sessionObserverStop(PyObject *self, PyObject *args){

	int ret = 0;
	M2M_T m2m;
	char *p_s_net = NULL, *p_s_s = NULL, *p_node = NULL;
	Py_ssize_t  net_len = 0, s_len = 0, node_len = 0;
	Py_callback_T *cb_arg = NULL;
	void *p_notify = NULL;
	// get net  
	if(!PyArg_ParseTuple( args,"s#s#s#:(net, session observer index).", &p_s_net, &net_len, &p_s_s, &s_len, &p_node, &node_len) ){
		PY_EXC_MSG_RETURN(PyExc_Exception, "get sessionObserverStop args error !", NULL);
	}
	PY_RETURN_UNEQUAL((int)net_len, (int)sizeof(size_t), PyExc_Exception, "sessionObserverStop can't find net !", NULL);
	mcpy(&m2m.net , p_s_net, net_len);	

	PY_RETURN_UNEQUAL((int)s_len, (int)sizeof(size_t), PyExc_Exception, "sessionObserverStopcan't find session !", NULL);
	mcpy(&m2m.session, p_s_s, s_len);

	PY_RETURN_UNEQUAL((int)node_len, (int)sizeof(size_t), PyExc_Exception, "sessionObserverStopcan't find notify pointer !", NULL);
	mcpy(&p_notify, p_node, node_len);
	
	ret = m2m_session_observer_stop(&m2m, p_notify);
	return Py_BuildValue("i",ret);

}
static PyObject *sessionNotifyPush(PyObject *self, PyObject *args){

	int ret = 0;
	M2M_T m2m;
	char *p_s_net = NULL,*p_data = NULL,*p_node = NULL;
	PyObject *py_cb = NULL, *py_cb_args = NULL;
	Py_ssize_t  net_len = 0, data_len = 0, node_len = 0;
	Py_callback_T *cb_arg = NULL;
	void *p_notify = NULL;
	// get net  
	if(!PyArg_ParseTuple( args,"s#s#s#|OO:(net, notify index, data, callback function, callback args).", &p_s_net, &net_len, &p_node, &node_len, &p_data, &data_len, &py_cb, &py_cb_args) ){
		PY_EXC_MSG_RETURN(PyExc_Exception, "get sessionNotifyPush args error !", NULL);
	}
	PY_RETURN_UNEQUAL((int)net_len, (int)sizeof(size_t), PyExc_Exception, "sessionNotifyPush can't find net !", NULL);
	mcpy(&m2m.net , p_s_net, net_len);	

	PY_RETURN_UNEQUAL((int)node_len, (int)sizeof(size_t), PyExc_Exception, "sessionNotifyPush't find notify pointer !", NULL);
	mcpy(&p_notify, p_node, node_len);

	if(py_cb){
		cb_arg = _obj_callback_get(py_cb, py_cb_args, 0);
		PY_RETURN_EQUAL(cb_arg, NULL, PyExc_Exception,"sessionNotifyPush get callback arg failt!", NULL);
	}
	ret = m2m_session_notify_push(&m2m, p_notify, (int)data_len, (u8*)p_data, pym2m_callback,(void*)cb_arg);
	//PY_RETURN_UNEQUSL_0(ret, PyExc_Exception, "notify push failt !!", NULL);
	return Py_BuildValue("i",ret);
}
static PyObject *netBroadcastStart(PyObject *self, PyObject *args){
	size_t net = 0;
	int ret, rport = 0;
	const char *py_net = NULL, *p_data=NULL;
	PyObject *py_cb, *py_cb_args = NULL;
	Py_ssize_t  net_len =0, data_len = 0;
	Py_callback_T *cb_arg = NULL;

	if( !PyArg_ParseTuple(args,"s#is#s#|OO:(net, remote port, data, callback function, callback args)",\
		&py_net, &net_len, &rport, &p_data, &data_len,&py_cb, &py_cb_args)){
		PY_EXC_MSG_RETURN( PyExc_Exception, "Get netBroadcastStart args error ", NULL);		
	}
	
	if( (int)net_len != (int)sizeof(size_t) )
		PY_EXC_MSG_RETURN( PyExc_Exception, "netBroadcastStart can't find net !", NULL);		
	mcpy(&net, py_net, net_len);

	if(py_cb){
		cb_arg = _obj_callback_get(py_cb, py_cb_args, 0);
		if(cb_arg == NULL)	
			return NULL;
	}
	ret = m2m_broadcast_data_start(net, rport, data_len, p_data,pym2m_callback, (void*)cb_arg);
	return Py_BuildValue("i",ret);	
}
static PyObject *netBroadcastStop(PyObject *self, PyObject *args){
	size_t net = 0;
	int ret = 0;
	const char *py_net = NULL;
	Py_ssize_t	net_len =0;

	if( !PyArg_ParseTuple(args,"s#:(net)",&py_net, &net_len)){
		PY_EXC_MSG_RETURN( PyExc_Exception, "Get netBroadcastStop args error ", NULL);		
	}
	
	if( (int)net_len != (int)sizeof(size_t) )
		PY_EXC_MSG_RETURN( PyExc_Exception, "netBroadcastStop can't find net !", NULL);		
	mcpy(&net, py_net, net_len);

	ret = m2m_broadcast_data_stop(net);
	return Py_BuildValue("i",ret);	
}
static PyObject *netBroadcastEnable(PyObject *self, PyObject *args){
	size_t net = 0;
	int ret = 0;
	const char *py_net = NULL;
	Py_ssize_t	net_len =0;

	if( !PyArg_ParseTuple(args,"s#:(net)",&py_net, &net_len)){
		PY_EXC_MSG_RETURN( PyExc_Exception, "Get netBroadcastEnable args error ", NULL);		
	}
	
	if( (int)net_len != (int)sizeof(size_t) )
		PY_EXC_MSG_RETURN( PyExc_Exception, "netBroadcastEnable can't find net !", NULL);		
	mcpy(&net, py_net, net_len);

	m2m_broadcast_enable(net);
	Py_RETURN_NONE;

}
static PyObject *netBroadcastDisable(PyObject *self, PyObject *args){
	size_t net = 0;
	int ret = 0;
	const char *py_net = NULL;
	Py_ssize_t	net_len =0;

	if( !PyArg_ParseTuple(args,"s#:(net)",&py_net, &net_len)){
		PY_EXC_MSG_RETURN( PyExc_Exception, "Get m2m_broadcast_disable args error ", NULL);		
	}
	
	if( (int)net_len != (int)sizeof(size_t) )
		PY_EXC_MSG_RETURN( PyExc_Exception, "m2m_broadcast_disable can't find net !", NULL);		
	mcpy(&net, py_net, net_len);

	m2m_broadcast_disable(net);
	Py_RETURN_NONE;
}
static PyObject *netOnlineCheck(PyObject *self, PyObject *args){

	size_t net = 0;
	int ret, rport = 0;
	const char *py_net = NULL, *p_rid = NULL,*p_rhost = NULL;
	PyObject *py_cb, *py_cb_args = NULL;
	M2M_id_T rid;
	Py_ssize_t	net_len =0, rid_len = 0, rhost_len = 0;
	Py_callback_T *cb_arg = NULL;

	mmemset(&rid,0, sizeof(M2M_id_T));
	if( !PyArg_ParseTuple(args,"s#s#s#is#s#|OO:(net,remote id, remote host,remote port,callback function, callback args)",\
		&py_net, &net_len, &p_rid, &rid_len, &p_rhost, &rhost_len, &rport, &py_cb, &py_cb_args)){
		PY_EXC_MSG_RETURN( PyExc_Exception, "Get netOnlineCheck args error ", NULL);		
	}
	if( (int)net_len != (int)sizeof(size_t) ){
		PY_EXC_MSG_RETURN( PyExc_Exception, "netOnlineCheck can't find net !", NULL);		
		}
	mcpy(&net, py_net, net_len);
	// get remote id 
	mcpy(rid.id,p_rid, M_MIN( (int)rid_len,ID_LEN));

	if(py_cb){
		cb_arg = _obj_callback_get(py_cb, py_cb_args, 0);
		if(cb_arg == NULL)	
			return NULL;
	}
	ret = m2m_dev_online_check(net, p_rhost, rport, &rid, pym2m_callback, (void*)cb_arg);
	return Py_BuildValue("i",ret);

}
/* regist api */
PyMethodDef pym2m_methods[] = {
  {
    "m2mInit",
    m2mInit,
    METH_VARARGS,
    "config m2m modle \t usage: .m2mInit( encode_tyepe, modle callback function, callback args, is_relay, max router time)\n "
    "\t encode_tyepe: 1-aes128,2-broadcast; modle callback function: callabck function;  callback args: callback arg.\n"
    "\t\t option: \t is_relay: 1-relay package,0-no relay package; max router time: max time "
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
    "usage: .netCreat( local id, local port, secret key, callback function, callback args, server id, server host, server's port)\n "
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
  {
    "sessionCreat",
    sessionCreat,
    METH_VARARGS,
    "sessionCreat  \t usage: .sessionCreat(net(bytes), remote id(bytes), remote host(string), \n"
    "\t\t remote secret key(string), remote prot(int), call back function(object), callback args(object).\n"
  },
  {
    "sessionDestory",
    sessionDestory,
    METH_VARARGS,
    "sessionCreat  \t usage: .sessionCreat(net(bytes), session(bytes) )\n"
    "description: destory an session. \n"
  },
   {
    "sessionSendData",
    sessionSendData,
    METH_VARARGS,
    "sessionSendData  \t usage: .sessionSendData(net(bytes), session(bytes), data to be send(byte), callback function(object), callback args(object))\n"
    "description: sending data to remote. \n"
  },  
  {
    "sessionTokenUpdate",
    sessionTokenUpdate,
    METH_VARARGS,
    "sessionTokenUpdate  \t usage: .sessionTokenUpdate(net(bytes), session(bytes),callback function(object), callback args(object))\n"
    "description: update session token( client token will be update, while master token no change.). \n"
  },
  {
    "sessionSecretkeySet",
    sessionSecretkeySet,
    METH_VARARGS,
    "sessionSecretkeySet  \t usage: .sessionSecretkeySet(net(bytes), session(bytes), key(bytes),callback function(object), callback args(object))\n"
    "description: update session secret key. \n"
  },
  {
    "netSecretkeySet",
    netSecretkeySet,
    METH_VARARGS,
	"netSecretkeySet  \t usage: .netSecretkeySet(net(bytes), remote id(bytes), remote host(string),remote prot(int), \n"
	"\t\t remote secret key(string), new secret key(string),  call back function(object), callback args(object).\n"
  },
  {
    "sessionObserverStart",
    sessionObserverStart,
    METH_VARARGS,
	"sessionObserverStart \t usage: .sessionObserverStart(net(bytes), session(bytes), ack type(int: 0-must ack, 1-no need to ack),\n"
	"\t\t\tdata(bytes),callback function(opton:object), callback args(opton:object))\n"
	"description: start an observer. \n"
  },
  {
    "sessionObserverStop",
    sessionObserverStop,
    METH_VARARGS,
	"sessionObserverStop  \t usage: .sessionObserverStop(net(bytes), session(bytes), notify index(bytes))\n"
	"description: stop observer. \n"
  },
   {
    "sessionNotifyPush",
    sessionNotifyPush,
    METH_VARARGS,
	"sessionNotifyPush  \t usage: .sessionNotifyPush(net(bytes), session(bytes), notify index(bytes), \n"
	"\t\t\tdata(string),  call back function(object), callback args(object).\n"
	"description: pushing an notify. \n"
  },
  {
    "netBroadcastStart",
    netBroadcastStart,
    METH_VARARGS,
	"netBroadcastStart  \t usage: .netBroadcastStart(net(bytes), remote port(int),\n"
	"\t\t\tdata(bytes),  call back function(object), callback args(object).\n"
	"description: start broad cast. \n"
  },
  {
    "netBroadcastStop",
    netBroadcastStop,
    METH_VARARGS,
	"netBroadcastStop  \t usage: .netBroadcastStop(net(bytes).\n"
	"description: sStop broad cast. \n"
  },
  {
    "netBroadcastEnable",
    netBroadcastEnable,
    METH_VARARGS,
	"netBroadcastEnable  \t usage: .netBroadcastEnable(net(bytes).\n"
	"description: Enable broad cast package receive. \n"
  },
  {
    "netBroadcastDisable",
    netBroadcastDisable,
    METH_VARARGS,
	"netBroadcastDisable  \t usage: .netBroadcastDisable(net(bytes).\n"
	"description: disable broad cast package receive. \n"
  },
  {
    "netOnlineCheck",
    netOnlineCheck,
    METH_VARARGS,
	"netOnlineCheck  \t usage: .netOnlineCheck(net(bytes), remore id, remote host, remote port, callback function, callback args)\n"
	"description: disable broad cast package receive. \n"
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

