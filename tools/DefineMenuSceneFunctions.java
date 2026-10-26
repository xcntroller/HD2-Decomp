// Seed fingerprint- and vtable-verified scene methods for read-only research.
// @category HD2

import ghidra.app.script.GhidraScript;
import ghidra.program.model.address.Address;
import ghidra.program.model.listing.Function;
import java.util.LinkedHashSet;
import java.util.Set;

public class DefineMenuSceneFunctions extends GhidraScript {
    @Override
    public void run() throws Exception {
        if (!"12c61eed2aec0c45700ad0a5ddfd7cbb3cbf0ea15ec67de155790bb24e7756ee"
                .equals(currentProgram.getExecutableSHA256())
                || currentProgram.getImageBase().getOffset() != 0x10000000L) {
            throw new IllegalArgumentException("Expected the original LS3DF.dll");
        }
        long[][] slots = {
            {0x1009b640L, 0x00, 0x100152f0L}, {0x1009b640L, 0x04, 0x10015650L},
            {0x1009b640L, 0x08, 0x10015560L}, {0x1009b640L, 0x54, 0x10019e10L},
            {0x1009b798L, 0x74, 0x10046820L}, {0x1009b798L, 0x78, 0x10046860L},
            {0x1009b798L, 0x80, 0x100469a0L}, {0x1009b798L, 0x90, 0x100471a0L},
            {0x1009ba50L, 0x00, 0x1001c370L}, {0x1009ba50L, 0x08, 0x1001c9f0L},
            {0x1009ba50L, 0x1c, 0x1001c400L}, {0x1009ba50L, 0x38, 0x1001d680L},
            {0x1009bc20L, 0x74, 0x1004ceb0L}, {0x1009b798L, 0x60, 0x100462b0L}
        };
        Set<Long> entries = new LinkedHashSet<>();
        for (long[] slot : slots) {
            if (Integer.toUnsignedLong(getInt(toAddr(slot[0] + slot[1]))) != slot[2]) {
                throw new IllegalStateException("Unexpected scene/frame vtable slot");
            }
            entries.add(slot[2]);
        }
        for (long table : new long[] {0x1009bd70L, 0x1009be08L}) {
            for (long offset : new long[] {8, 0x74, 0x7c, 0x84, 0x88, 0x90}) {
                long entry = Integer.toUnsignedLong(getInt(toAddr(table + offset)));
                if (entry < 0x10001000L || entry >= 0x1009b000L) {
                    throw new IllegalStateException("Non-code frame target");
                }
                entries.add(entry);
            }
        }
        for (long entry : entries) {
            Address address = toAddr(entry);
            if (getFunctionAt(address) == null) {
                Function containing = getFunctionContaining(address);
                if (containing != null) {
                    throw new IllegalStateException(address + " overlaps " + containing.getEntryPoint());
                }
                if (getInstructionAt(address) == null && !disassemble(address)) {
                    throw new IllegalStateException("Cannot disassemble " + address);
                }
                if (createFunction(address, null) == null) {
                    throw new IllegalStateException("Cannot define " + address);
                }
            }
            println("Menu scene target: " + address);
        }
    }
}
