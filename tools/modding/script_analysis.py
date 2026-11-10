"""Structured, evidence-labelled views over actual recovered compiler products."""

SIMPLE_SLOTS={0x55:0,0x56:1,0x57:3,0x58:2,0x59:6,0x5a:7,0x5b:4}
NUMERIC={0x53:'signals',0xa4:'cutscenes',0x5c:'cutscene_done'}


def enrich_script(report,mapping):
    def locate(item):
        if 'span' in item:
            item['locations']=mapping.locations(item['span']);item['span_space']='prepared_source'
    for diagnostic in report['diagnostics']:
        diagnostic['origin']='tool_restriction' if diagnostic['category'] in ('source_limit','embedded_nul','nesting_limit','operator_limit') else 'recovered_frontend_rejection'
    for key in ('tokens','statements','arguments','diagnostics','lexical_mentions'):
        for item in report[key]:locate(item)
    statements=report['statements'];instructions=report['instructions']
    by_pc={i['pc']:i for i in instructions}
    arguments={}
    for n,arg in enumerate(report['arguments']):
        arg['index']=n
        arg['text']=mapping.data[arg['span']['start']:arg['span']['end']].decode('latin-1')
        arguments.setdefault(arg['statement'],[]).append(arg)
    report.update(events=[],declarations=[],labels=[],symbol_references=[],controls=[])
    tokens=report['tokens'];ti=0
    for statement in statements:
        op=statement['opcode'];index=statement['index'];a,b=statement['span']['start'],statement['span']['end']
        while ti<len(tokens) and tokens[ti]['span']['end']<=a:ti+=1
        local=[];n=ti
        while n<len(tokens) and tokens[n]['span']['start']<b:
            local.append(tokens[n]);n+=1
        statement['argument_indices']=[arg['index'] for arg in arguments.get(index,[])]
        statement['identity']=f"{report['file']}#statement={index}"
        base={'statement':index,'span':statement['span'],'locations':statement['locations'],
              'pc_begin':statement['pc_begin'],'pc_end':statement['pc_end'],'span_space':'prepared_source','parse_accepted':statement['parse_accepted'],'compiled':report['syntax_accepted']}
        first=by_pc.get(statement['pc_begin'],{}) if statement['pc_begin'] is not None and statement['pc_end']>statement['pc_begin'] else {}
        if op in (0x46,0x49,0x4b,0x48,0x4a) and len(local)>1:
            declaration={**base,'name':local[1]['text'],'kind':{0x46:2,0x49:4,0x4b:5}.get(op),
                         'supported':op in (0x46,0x49,0x4b),'evidence':'0042ECD0'}
            report['declarations'].append(declaration)
        if op==0x4c and len(local)>1:
            report['labels'].append({**base,'name':local[1]['text'],'target_pc':statement['pc_begin'],'evidence':'0042F0A0'})
        if op in (0x4d,0x51,0x4e,0x4f,0x52,0x105,0x106,0x112) or first.get('opcode')==0x107:
            control={**base,'kind':statement['name'],'operands':first.get('operands',[]),'evidence':'0042C0B0'}
            if op in (0x4d,0x51):control['target_pc']=first.get('operands',[None])[0]
            if first.get('opcode')==0x107:
                operands=first['operands'];control['kind']='assignment'
                control['expression_index']=operands[0]
                control['variable']=next((v for v in report['variables'] if v['kind']==operands[1] and v['pool_index']==operands[2]),None)
            report['controls'].append(control)
        if not report['syntax_accepted']:continue
        pc=statement['pc_begin'];first=by_pc.get(pc)
        if op in SIMPLE_SLOTS or op in NUMERIC or op in (0x50,0x54):
            event={**base,'name':statement['name'],'entry_pc':pc if op==0x54 else pc+1,
                   'registered':False,'evidence':'0042E410/0042E730/0042E460/0042E550/0042E640/0042E7C0'}
            if op in SIMPLE_SLOTS:
                event['slot']=SIMPLE_SLOTS[op]
                event['registered']=report['event_slots'][event['slot']]==pc+1
            elif op in NUMERIC:
                event['identity']=first['operands'][0];event['table']=NUMERIC[op]
                event['registered']=any(e['identity']==event['identity'] and e['entry_pc']==pc+1 for e in report['event_tables'][event['table']])
            elif op==0x50:
                event['symbol']=local[1]['text']
                event['record']=next(w for w in report['whenevers'] if w['entry_pc']==pc+1)
                event['registered']=True
            else:event['registered']=report['start_pc']==pc
            entry=by_pc.get(event['entry_pc'])
            if entry and entry['opcode']==0x105:
                event['block_open_pc']=entry['pc'];event['block_close_pc']=entry['pc']+entry['operands'][0]+1
            report['events'].append(event)
        # Symbol references are compiler-proven ordinary argument uses only.
        for arg in arguments.get(index,[]):
            symbol=next((v for v in report['variables'] if v['kind']==arg['kind'] and v['pool_index']==arg['pool_index'] and v['name'].lower()==arg['text'].lower()),None)
            if symbol:report['symbol_references'].append({**arg,'name':symbol['name'],'evidence':'0042D890/0042D920'})
    for instruction in instructions:
        index=instruction['statement']
        if index is not None:
            instruction.update(span=statements[index]['span'],locations=statements[index]['locations'],span_space='prepared_source')
    for reference in report['references']:
        instruction=by_pc[reference['pc']];index=instruction['statement']
        arg=next((a for a in arguments.get(index,[]) if a['slot']==1),None)
        if arg:reference.update(span=arg['span'],locations=arg['locations'],span_space='prepared_source')
        reference['evidence']='00416E10 opcodes 1/5E/66 string operand 1'
        reference['instruction_identity']=f"{report['file']}#pc={reference['pc']}"
    import hashlib
    report['prepared_source']={'size':len(mapping.data),'sha256':hashlib.sha256(mapping.data).hexdigest()}
    report['sources']=mapping.metadata()
    report['source_map']=[{'start':p.start,'end':p.end,'source':p.source,'origin':p.origin} for p in mapping.pieces]
    report['limitations'] += [
        'Events/PCs and ordinary argument pools are recovered compiler products, not proof of execution.',
        'Expression syntax uses original grouping quirks. Expression trees/data-flow and predicate argument spans are not exposed.',
        'Symbol reference list covers ordinary command arguments only; assignment/predicate uses remain uninterpreted.',
        'Spans are half-open byte ranges; displayed line/column positions are 1-based with CRLF/CR/LF normalized, unlike native CR-only counters.',
        'On compiler failure only observed tokens and partial statement syntax are retained; no compiled PC/event/symbol claims.']
    return report
