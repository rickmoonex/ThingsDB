import re
import sys
from pyleri import (
    Grammar,
    Keyword,
    Regex,
    Choice as Choice_,
    Sequence,
    Ref,
    Prio,
    Token,
    Tokens,
    Repeat,
    List as List_,
    Optional,
    THIS,
)

RE_NAME = r'^[A-Za-z_][0-9A-Za-z_]*'


class Choice(Choice_):
    def __init__(self, *args, most_greedy=None, **kwargs):
        if most_greedy is None:
            most_greedy = False
        super().__init__(*args, most_greedy=most_greedy, **kwargs)


class List(List_):
    def __init__(self, *args, opt=None, **kwargs):
        if opt is None:
            opt = True
        super().__init__(*args, opt=opt, **kwargs)


class Definition(Grammar):
    RE_KEYWORDS = re.compile(RE_NAME)

    r_single_quote = Regex(r"(?:'(?:[^']*)')+")
    r_double_quote = Regex(r'(?:"(?:[^"]*)")+')

    t_false = Keyword('false')
    t_float = Regex(r'[-+]?[0-9]*\.?[0-9]+')
    t_int = Regex(r'[-+]?[0-9]+')
    t_nil = Keyword('nil')
    t_string = Choice(r_single_quote, r_double_quote)
    t_true = Keyword('true')
    t_undefined = Keyword('undefined')

    o_not = Repeat(Token('!'))
    comment = Repeat(Regex(r'(?s)/\\*.*?\\*/'))

    name = Regex(RE_NAME)

    # build-in get functions
    f_blob = Keyword('blob')
    f_endswith = Keyword('endswith')
    f_filter = Keyword('filter')
    f_get = Keyword('get')
    f_id = Keyword('id')
    f_lower = Keyword('lower')
    f_map = Keyword('map')
    f_ret = Keyword('ret')
    f_startswith = Keyword('startswith')
    f_thing = Keyword('thing')
    f_upper = Keyword('upper')

    # build-in update functions
    f_del = Keyword('del')
    f_new = Keyword('new')
    f_push = Keyword('push')
    f_rename = Keyword('rename')
    f_set = Keyword('set')
    f_splice = Keyword('splice')
    f_unset = Keyword('unset')

    primitives = Choice(
        t_false,
        t_nil,
        t_true,
        t_undefined,
        t_int,
        t_float,
        t_string,
        most_greedy=True,
    )

    scope = Ref()
    chain = Ref()

    thing = Sequence('{', List(Sequence(name, ':', scope)), '}')
    array = Sequence('[', List(scope), ']')

    arrow = Sequence(List(name, opt=False), '=>', scope)

    function = Sequence(Choice(
        # build-in get functions
        f_blob,         # (int inx_in_blobs) -> raw
        f_endswith,     # (str) -> bool
        f_filter,       # (arrow) -> [return values where return is true]
        f_get,          # (str,..) -> attribute val
        f_id,           # () -> int
        f_lower,        # () -> str
        f_map,          # (arrow) -> [return values]
        f_ret,          # () -> nil
        f_startswith,   # (str) -> bool
        f_thing,        # (int thing_id) -> thing
        f_upper,        # () -> str
        # build-in update functions
        f_del,
        f_new,
        f_push,
        f_rename,
        f_set,
        f_splice,
        f_unset,
        # any name
        name,
    ), '(', List(scope), ')')

    opr0_mul_div_mod = Sequence(THIS, Tokens('* / % //'), THIS)
    opr1_add_sub = Sequence(THIS, Tokens('+ -'), THIS)
    opr2_compare = Sequence(THIS, Tokens('< > == != <= >='), THIS)
    opr3_cmp_and = Sequence(THIS, '&&', THIS)
    opr4_cmp_or = Sequence(THIS, '||', THIS)

    operations = Sequence(
        '(',
        Prio(
            scope,
            opr0_mul_div_mod,
            opr1_add_sub,
            opr2_compare,
            opr3_cmp_and,  # we could add here + - * / etc.
            opr4_cmp_or,
        ),
        ')',
    )

    assignment = Sequence(name, Tokens('= += -= *= /= %='), scope)
    index = Repeat(
        Sequence('[', t_int, ']')
    )       # we skip index in query investigate (in case we want to use scope)

    chain = Sequence(
        '.',
        Choice(function, assignment, name),
        index,
        Optional(chain),
    )

    scope = Sequence(
        o_not,
        Choice(
            primitives,
            function,
            assignment,
            arrow,
            name,
            thing,
            array,
            operations,
        ),
        index,
        Optional(chain),
    )

    START = Sequence(
        comment,
        List(scope, delimiter=Sequence(';', comment)),
    )

    @classmethod
    def translate(cls, elem):
        if elem == cls.name:
            return 'name'

    def test(self, str):
        print('{} : {}'.format(
            str.strip(), self.parse(str).as_str(self.translate)))


if __name__ == '__main__':
    definition = Definition()
    definition.test('users.find(user => (user.id == 1)).labels.filter(label => (label.id().i == 1))')
    definition.test('users.find(user => (user.id == 1)).labels.filter(label => (label.id().i == 1))')
    # exit(0)
    definition.test('users.create("iris");grant(users.iris,FULL)')
    definition.test('labels.map(label => label.id())')
    definition.test('''
        /*
         * Create a database
         */
        databases.create(dbtest);

        /*
         * Drop a database
         */
        databases.dbtest.drop();

        /* Change redundancy */
        config.redundancy = 3[0][0];

        types().create(User, {
            name: str().required(),
            age: int().required(),
            owner: User.required(),
            schools: School.required(),
            scores: thing,
            other: int
        });
        users.new({
            name: 'iris'
        });

        type().add(users, User.isRequired);

        bla.set('x', 4);

        /*
         * Finished!
         */
    ''')

    definition.test(' users.new({name: "iris"}); ')
    definition.test(' databases.dbtest.drop() ')

    definition.test('2.1')

    {
        '$ev': 0,
        '#': 4,
        '$jobs': [
            {'assign': {'age', 5}},
            {'del': 'age'},
            {'set': {'name': 'iris'}},
            {'set': {'image': '<bin_data>'}},
            {'unset': 'name'},
            {'push': {'people': [{'#': 123}]}}
        ]
    }

    {
        '$ev': 1,
        '#': 0,
        '$jobs': [
            {'new': {'Database': {
                'name': 'testdb',
                'user': 'iris'
            }}}
        ]
    }

    c, h = definition.export_c(target='langdef', headerf='<langdef/langdef.h>')
    with open('../src/langdef/langdef.c', 'w') as cfile:
        cfile.write(c)

    with open('../inc/langdef/langdef.h', 'w') as hfile:
        hfile.write(h)

    print('Finished export to c')