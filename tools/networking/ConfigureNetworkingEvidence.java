// Transient decompiler models; use -readOnly -noanalysis. No original bytes change.
// @category HD2
import ghidra.app.script.GhidraScript;
import ghidra.program.model.data.*;
import ghidra.program.model.listing.*;
import ghidra.program.model.symbol.SourceType;

public class ConfigureNetworkingEvidence extends GhidraScript {
    private void signature(long address, String convention, DataType result, DataType... args) throws Exception {
        Function f = getFunctionAt(toAddr(address));
        if (f == null) throw new IllegalStateException("Missing function " + Long.toHexString(address));
        Parameter[] parameters = new Parameter[args.length];
        for (int i = 0; i < args.length; ++i) parameters[i] = new ParameterImpl("arg" + i, args[i], currentProgram);
        f.updateFunction(convention, new ReturnParameterImpl(result, currentProgram),
            Function.FunctionUpdateType.DYNAMIC_STORAGE_ALL_PARAMS, true, SourceType.USER_DEFINED, parameters);
    }
    public void run() throws Exception {
        String hash = currentProgram.getExecutableSHA256();
        DataType ptr = new PointerDataType(ByteDataType.dataType, 4);
        if (hash.equals("6ed896ce1b468b04ce3d0136f9d4b51340c648838bd9a2eb34761cc2a1c96fbb")) {
            // 4116A6/B8 caller pops 8 for C-string/string insertion; 4117AC sets ECX
            // for numeric member insertion. Existing cdecl numeric model corrupts ESP.
            signature(0x40e100, "__cdecl", ptr, ptr, ptr);
            signature(0x40e380, "__cdecl", ptr, ptr, ptr);
            for (long a : new long[]{0x40e750, 0x40e8e0, 0x40ea70})
                signature(a, "__thiscall", ptr, DWordDataType.dataType);
            byte[] prefix = {(byte)0x3d,0,0x10,0,0,0x73,0x0e};
            for (int i=0;i<prefix.length;++i) if (getByte(toAddr(0x414910L+i))!=prefix[i])
                throw new IllegalStateException("Launcher stack probe mismatch");
            getFunctionAt(toAddr(0x414910)).setCallFixup("alloca_probe");
        } else if (hash.equals("48ba25814ab429fed4418d8610b2eca5aaeb594d83a29a0ca6b99fa152f1c152")) {
            byte[] prefix = {0x51,0x3d,0,0x10,0,0,(byte)0x8d,0x4c,0x24,0x08};
            for (int i=0;i<prefix.length;++i) if (getByte(toAddr(0x822460L+i))!=prefix[i])
                throw new IllegalStateException("Server stack probe mismatch");
            getFunctionAt(toAddr(0x822460)).setCallFixup("alloca_probe");
            // ECX cursor**, EDX text buffer; stack integer out and keyword table.
            signature(0x7a47b0, "__fastcall", DWordDataType.dataType, ptr, ptr, ptr, ptr);
            signature(0x40d46d, "__fastcall", DWordDataType.dataType, ptr, ptr, ptr, ptr);
            //006F03A6 saves ECX as the profile owner; name is one stack arg,
            //006F05C9 RET4. Without this model the decompiler invents an
            //uninitialized local24 pointer for the real owner.
            signature(0x6f0380, "__thiscall", ByteDataType.dataType, ptr);
            signature(0x409d1d, "__thiscall", ByteDataType.dataType, ptr);
        } else throw new IllegalArgumentException("Unsupported networking fingerprint");
    }
}
