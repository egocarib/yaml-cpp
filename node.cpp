#include "node.h"
#include "token.h"
#include "scanner.h"
#include "content.h"
#include "parser.h"
#include "scalar.h"
#include "sequence.h"
#include "map.h"

namespace YAML
{
	Node::Node(): m_pContent(0), m_alias(false)
	{
	}

	Node::~Node()
	{
		Clear();
	}

	void Node::Clear()
	{
		delete m_pContent;
		m_pContent = 0;
		m_alias = false;
	}

	void Node::Parse(Scanner *pScanner, const ParserState& state)
	{
		Clear();

		ParseHeader(pScanner, state);

		// is this an alias? if so, it can have no content
		if(m_alias)
			return;

		// now split based on what kind of node we should be
		Token *pToken = pScanner->PeekNextToken();
		if(pToken->type == TT_DOC_END)
			return;

		switch(pToken->type) {
			case TT_SCALAR:
				m_pContent = new Scalar;
				m_pContent->Parse(pScanner, state);
				break;
			case TT_FLOW_SEQ_START:
			case TT_BLOCK_SEQ_START:
			case TT_BLOCK_ENTRY:
				m_pContent = new Sequence;
				m_pContent->Parse(pScanner, state);
				break;
			case TT_FLOW_MAP_START:
			case TT_BLOCK_MAP_START:
				m_pContent = new Map;
				m_pContent->Parse(pScanner, state);
		}
	}

	// ParseHeader
	// . Grabs any tag, alias, or anchor tokens and deals with them.
	void Node::ParseHeader(Scanner *pScanner, const ParserState& state)
	{
		while(1) {
			Token *pToken = pScanner->PeekNextToken();
			if(!pToken || (pToken->type != TT_TAG && pToken->type != TT_ANCHOR && pToken->type != TT_ALIAS))
				break;

			switch(pToken->type) {
				case TT_TAG: ParseTag(pScanner, state); break;
				case TT_ANCHOR: ParseAnchor(pScanner, state); break;
				case TT_ALIAS: ParseAlias(pScanner, state); break;
			}
		}
	}

	void Node::ParseTag(Scanner *pScanner, const ParserState& state)
	{
		if(m_tag != "")
			return;  // TODO: throw

		Token *pToken = pScanner->PeekNextToken();
		m_tag = state.TranslateTag(pToken->value);

		for(unsigned i=0;i<pToken->params.size();i++)
			m_tag += pToken->params[i];
		pScanner->PopNextToken();
	}
	
	void Node::ParseAnchor(Scanner *pScanner, const ParserState& state)
	{
		if(m_anchor != "")
			return;  // TODO: throw

		Token *pToken = pScanner->PeekNextToken();
		m_anchor = pToken->value;
		m_alias = false;
		pScanner->PopNextToken();
	}

	void Node::ParseAlias(Scanner *pScanner, const ParserState& state)
	{
		if(m_anchor != "")
			return;  // TODO: throw
		if(m_tag != "")
			return;  // TODO: throw (aliases can't have any content, *including* tags)

		Token *pToken = pScanner->PeekNextToken();
		m_anchor = pToken->value;
		m_alias = true;
		pScanner->PopNextToken();
	}

	void Node::Write(std::ostream& out, int indent)
	{
		if(m_tag != "") {
			for(int i=0;i<indent;i++)
				out << "  ";
			out << "{tag: " << m_tag << "}\n";
		}
		if(m_anchor != "") {
			for(int i=0;i<indent;i++)
				out << "  ";
			if(m_alias)
				out << "{alias: " << m_anchor << "}\n";
			else
				out << "{anchor: " << m_anchor << "}\n";
		}

		if(!m_pContent) {
			for(int i=0;i<indent;i++)
				out << "  ";
			out << "{no content}\n";
		} else {
			m_pContent->Write(out, indent);
		}
	}

	// begin
	// Returns an iterator to the beginning of this (sequence or map).
	Node::Iterator Node::begin() const
	{
		if(!m_pContent)
			return Iterator();

		std::vector <Node *>::const_iterator seqIter;
		if(m_pContent->GetBegin(seqIter))
			return Iterator(seqIter);

		std::map <Node *, Node *>::const_iterator mapIter;
		if(m_pContent->GetBegin(mapIter))
			return Iterator(mapIter);

		return Iterator();
	}

	// end
	// . Returns an iterator to the end of this (sequence or map).
	Node::Iterator Node::end() const
	{
		if(!m_pContent)
			return Iterator();

		std::vector <Node *>::const_iterator seqIter;
		if(m_pContent->GetEnd(seqIter))
			return Iterator(seqIter);

		std::map <Node *, Node *>::const_iterator mapIter;
		if(m_pContent->GetEnd(mapIter))
			return Iterator(mapIter);

		return Iterator();
	}

	///////////////////////////////////////////////////////
	// Extraction

	void operator >> (const Node& node, std::string& s)
	{
		if(!node.m_pContent)
			throw;

		node.m_pContent->Read(s);
	}

	void operator >> (const Node& node, int& i)
	{
		if(!node.m_pContent)
			throw;

		node.m_pContent->Read(i);
	}

	void operator >> (const Node& node, unsigned& u)
	{
		if(!node.m_pContent)
			throw;

		node.m_pContent->Read(u);
	}

	void operator >> (const Node& node, long& l)
	{
		if(!node.m_pContent)
			throw;

		node.m_pContent->Read(l);
	}

	void operator >> (const Node& node, float& f)
	{
		if(!node.m_pContent)
			throw;

		node.m_pContent->Read(f);
	}

	void operator >> (const Node& node, double& d)
	{
		if(!node.m_pContent)
			throw;

		node.m_pContent->Read(d);
	}

	void operator >> (const Node& node, char& c)
	{
		if(!node.m_pContent)
			throw;

		node.m_pContent->Read(c);
	}
}
