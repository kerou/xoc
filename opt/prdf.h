/*@
XOC Release License

Copyright (c) 2013-2014, Alibaba Group, All rights reserved.

    compiler@aliexpress.com

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright
      notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright
      notice, this list of conditions and the following disclaimer in the
      documentation and/or other materials provided with the distribution.
    * Neither the name of the Su Zhenyu nor the names of its contributors
      may be used to endorse or promote products derived from this software
      without specific prior written permission.

THIS SOFTWARE IS PROVIDED "AS IS" AND ANY EXPRESS OR IMPLIED
WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS
BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT
OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR
BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN
IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

author: Su Zhenyu
@*/
#ifndef __PRDF_H__
#define __PRDF_H__

namespace xoc {

//Map between VAR and PRno.
class VAR2PR : public TMap<VAR const*, UINT, CompareConstVar> {
};

class PRDF : public Pass {
	Region * m_ru;
	MDSystem * m_md_sys;
	VAR2PR * m_var2pr;
	IR_CFG * m_cfg;
	DefMiscBitSetMgr m_sbs_mgr;
	Vector<DefSBitSetCore*> m_def;
	Vector<DefSBitSetCore*> m_use;
	Vector<DefSBitSetCore*> m_livein;
	Vector<DefSBitSetCore*> m_liveout;
	BYTE m_handle_may:1; //true if consider maydef/mayuse info.

protected:
	DefSBitSetCore * get_def(UINT bbid)
	{
		DefSBitSetCore * set = m_def.get(bbid);
		if (set == NULL) {
			set = m_sbs_mgr.create_sbitsetc();
			m_def.set(bbid, set);
		}
		return set;
	}

	DefSBitSetCore * get_use(UINT bbid)
	{
		DefSBitSetCore * set = m_use.get(bbid);
		if (set == NULL) {
			set = m_sbs_mgr.create_sbitsetc();
			m_use.set(bbid, set);
		}
		return set;
	}

	inline void processOpnd(
					IR const* exp,
					List<IR const*> & lst,
					DefSBitSetCore * use,
					DefSBitSetCore * gen);
	inline void processMay(
					IR const* pr,
					DefSBitSetCore * gen,
					DefSBitSetCore * use,
					bool is_lhs);
public:
	PRDF(Region * ru)
	{
		m_ru = ru;
		m_md_sys = ru->get_md_sys();
		m_cfg = ru->get_cfg();
		m_var2pr = NULL;
		m_handle_may = false;
	}
	COPY_CONSTRUCTOR(PRDF);
	~PRDF()
	{
		for (INT i = 0; i <= m_def.get_last_idx(); i++) {
			DefSBitSetCore * bs = m_def.get((UINT)i);
			if (bs != NULL) {
				m_sbs_mgr.free_sbitsetc(bs);
			}
		}

		for (INT i = 0; i <= m_use.get_last_idx(); i++) {
			DefSBitSetCore * bs = m_use.get((UINT)i);
			if (bs != NULL) {
				m_sbs_mgr.free_sbitsetc(bs);
			}
		}

		for (INT i = 0; i <= m_livein.get_last_idx(); i++) {
			DefSBitSetCore * bs = m_livein.get((UINT)i);
			if (bs != NULL) {
				m_sbs_mgr.free_sbitsetc(bs);
			}
		}

		for (INT i = 0; i <= m_liveout.get_last_idx(); i++) {
			DefSBitSetCore * bs = m_liveout.get((UINT)i);
			if (bs != NULL) {
				m_sbs_mgr.free_sbitsetc(bs);
			}
		}
	}

	void computeLocal(IRBB * bb, List<IR const*> & lst);
	void computeGlobal();

	UINT count_mem() const
	{
		UINT count = m_sbs_mgr.count_mem();
		count += m_def.count_mem();
		count += m_use.count_mem();
		count += m_livein.count_mem();
		count += m_liveout.count_mem();

		for (INT i = 0; i <= m_def.get_last_idx(); i++) {
			DefSBitSetCore * bs = m_def.get((UINT)i);
			if (bs != NULL) {
				count += bs->count_mem();
			}
		}

		for (INT i = 0; i <= m_use.get_last_idx(); i++) {
			DefSBitSetCore * bs = m_use.get((UINT)i);
			if (bs != NULL) {
				count += bs->count_mem();
			}
		}

		for (INT i = 0; i <= m_livein.get_last_idx(); i++) {
			DefSBitSetCore * bs = m_livein.get((UINT)i);
			if (bs != NULL) {
				count += bs->count_mem();
			}
		}

		for (INT i = 0; i <= m_liveout.get_last_idx(); i++) {
			DefSBitSetCore * bs = m_liveout.get((UINT)i);
			if (bs != NULL) {
				count += bs->count_mem();
			}
		}

		return count;
	}

	void dump();

	virtual CHAR const* get_pass_name() const { return "PRDF"; }
	PASS_TYPE get_pass_type() const { return PASS_PRDF; }

	DefMiscBitSetMgr & getMiscBitSetMgr() { return m_sbs_mgr; }

	//Get livein PR. The return set is readonly.
	DefSBitSetCore const* get_livein_c(UINT bbid) const
	{ return m_livein.get(bbid); }

	//Get livein PR.
	DefSBitSetCore * get_livein(UINT bbid)
	{
		DefSBitSetCore * x = m_livein.get(bbid);
		if (x == NULL) {
			x = m_sbs_mgr.create_sbitsetc();
			m_livein.set(bbid, x);
		}
		return x;
	}

	//Get liveout PR. The return set is readonly.
	DefSBitSetCore const* get_liveout_c(UINT bbid) const
	{ return m_liveout.get(bbid); }

	//Get liveout PR. The return set is readonly.
	DefSBitSetCore * get_liveout(UINT bbid)
	{
		DefSBitSetCore * x = m_liveout.get(bbid);
		if (x == NULL) {
			x = m_sbs_mgr.create_sbitsetc();
			m_liveout.set(bbid, x);
		}
		return x;
	}

	//Handle may def/use.
	void set_handle_may(bool is_handle_may)
	{ m_handle_may = (BYTE)is_handle_may; }

	//Set map structure.
	void setVAR2PR(VAR2PR * v2p) { m_var2pr = v2p; }

	void setPRToBeLiveout(IRBB * bb, UINT prno)
	{ get_liveout(BB_id(bb))->bunion(prno, m_sbs_mgr); }

	virtual bool perform(OptCTX & oc);
};

} //namespace xoc
#endif
