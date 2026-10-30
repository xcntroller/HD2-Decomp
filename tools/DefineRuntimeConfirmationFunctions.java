// Verify indirect runtime-confirmation methods before read-only exports.
// @category HD2
import ghidra.app.script.GhidraScript;
import ghidra.program.model.address.Address;
import ghidra.program.model.listing.Function;
import java.util.LinkedHashSet;
import java.util.Set;

public class DefineRuntimeConfirmationFunctions extends GhidraScript {
    @Override
    public void run() throws Exception {
        if (!"8c52ce664091d0fcf090b1af5c1f085e055a645ae4cb497227184ed36f10b307"
                .equals(currentProgram.getExecutableSHA256())
                || currentProgram.getImageBase().getOffset() != 0x400000L)
            throw new IllegalArgumentException("Expected fingerprinted base client");
        Set<Long> entries = new LinkedHashSet<>();
        long[][] slots = {{0x20, 0x6691b0}, {0x80, 0x66c3b0}, {0x8c, 0x668790}};
        for (long[] slot : slots) {
            if (Integer.toUnsignedLong(getInt(toAddr(0x8155d8L + slot[0]))) != slot[1])
                throw new IllegalStateException("Unexpected runtime-confirmation vtable");
            entries.add(slot[1]);
        }
        long[][] branches = {{0x66d78aL, 0x6688a0L}, {0x66d7aaL, 0x668cd0L},
                             {0x66d7caL, 0x669090L}};
        for (long[] branch : branches) {
            Address site = toAddr(branch[0]);
            if (getByte(site) != (byte)0xe9 || branch[0] + 5 + getInt(site.add(1)) != branch[1])
                throw new IllegalStateException("Unexpected confirmation forwarding branch");
            entries.add(branch[1]);
        }
        for (long site : new long[]{0x6687e2L, 0x668821L, 0x66886fL}) {
            if (getByte(toAddr(site)) != 0x68 || getInt(toAddr(site + 1)) != 0x6693a0)
                throw new IllegalStateException("Unexpected confirmation observer");
        }
        entries.add(0x6693a0L);
        for (long entry : entries) {
            Address address = toAddr(entry);
            Function function = getFunctionAt(address);
            if (function == null) {
                Function containing = getFunctionContaining(address);
                if (containing != null)
                    throw new IllegalStateException(address + " overlaps " + containing.getEntryPoint());
                if (getInstructionAt(address) == null && !disassemble(address))
                    throw new IllegalStateException("Cannot disassemble " + address);
                function = createFunction(address, null);
                if (function == null) throw new IllegalStateException("Cannot define " + address);
            }
            println("Runtime confirmation target: " + address);
        }
    }
}
