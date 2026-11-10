/** Visual geometry only. No ranks from this module represent scheduling claims. */
export function relationshipLayout(
  ids: string[],
  links: { source: string; target: string }[],
): Map<string, { x: number; y: number }> {
  const ordered = [...new Set(ids)].sort();
  const adjacency = new Map(ordered.map(id => [id, new Set<string>()]));
  for (const { source, target } of links) {
    if (adjacency.has(source) && adjacency.has(target)) adjacency.get(source)!.add(target);
  }

  // Collapse cycles, including recursive calls, before layering the directed graph.
  let nextIndex = 0;
  const indices = new Map<string, number>();
  const low = new Map<string, number>();
  const stack: string[] = [];
  const active = new Set<string>();
  const components: string[][] = [];
  function visit(id: string) {
    indices.set(id, nextIndex);
    low.set(id, nextIndex++);
    stack.push(id);
    active.add(id);
    for (const target of [...adjacency.get(id)!].sort()) {
      if (!indices.has(target)) {
        visit(target);
        low.set(id, Math.min(low.get(id)!, low.get(target)!));
      } else if (active.has(target)) {
        low.set(id, Math.min(low.get(id)!, indices.get(target)!));
      }
    }
    if (low.get(id) === indices.get(id)) {
      const members: string[] = [];
      let member: string;
      do {
        member = stack.pop()!;
        active.delete(member);
        members.push(member);
      } while (member !== id);
      components.push(members.sort());
    }
  }
  for (const id of ordered) if (!indices.has(id)) visit(id);
  const owner = new Map(components.flatMap((members, i) => members.map(id => [id, i] as const)));
  const outgoing = components.map(() => new Set<number>());
  const incoming = components.map(() => new Set<number>());
  for (const [source, targets] of adjacency) for (const target of targets) {
    const a = owner.get(source)!;
    const b = owner.get(target)!;
    if (a !== b) { outgoing[a].add(b); incoming[b].add(a); }
  }
  const indegrees = incoming.map(set => set.size);
  const ranks = components.map(() => 0);
  const queue = components.map((_, i) => i).filter(i => !indegrees[i]);
  for (let cursor = 0; cursor < queue.length; cursor++) {
    const source = queue[cursor];
    for (const target of outgoing[source]) {
      ranks[target] = Math.max(ranks[target], ranks[source] + 1);
      if (--indegrees[target] === 0) queue.push(target);
    }
  }

  const layers = new Map<number, number[]>();
  const isolated: string[] = [];
  components.forEach((members, i) => {
    if (!incoming[i].size && !outgoing[i].size && members.length === 1) isolated.push(...members);
    else layers.set(ranks[i], [...(layers.get(ranks[i]) ?? []), i]);
  });
  const rowPositions = new Map<number, number>();
  const result = new Map<string, { x: number; y: number }>();
  let height = 0;
  for (const [rank, layer] of [...layers].sort(([a], [b]) => a - b)) {
    // Keep converging branches near their predecessors, with stable ID tie breaks.
    const center = (i: number) => {
      const parents = [...incoming[i]].map(p => rowPositions.get(p) ?? 0);
      return parents.length ? parents.reduce((a, b) => a + b, 0) / parents.length : 0;
    };
    layer.sort((a, b) => center(a) - center(b) || components[a][0].localeCompare(components[b][0]));
    let row = 0;
    for (const component of layer) {
      rowPositions.set(component, row + (components[component].length - 1) / 2);
      for (const id of components[component]) result.set(id, { x: rank * 390, y: row++ * 150 });
    }
    height = Math.max(height, row * 150);
  }
  // Disconnected records remain browseable without stretching fit-to-view into a single column.
  const columns = Math.min(5, Math.max(1, Math.ceil(Math.sqrt(isolated.length))));
  isolated.sort().forEach((id, i) => result.set(id, {
    x: (i % columns) * 390,
    y: (height ? height + 100 : 0) + Math.floor(i / columns) * 150,
  }));
  return result;
}
