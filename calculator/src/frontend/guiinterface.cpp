#include "guiinterface.h"


#include <QGridLayout>
#include <QLineEdit>
#include <QtMath>


Button::Button(const QString &text, QWidget *parent)
    : QToolButton(parent)
{
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
    setText(text);
}

QSize Button::sizeHint() const
{
    QSize size = QToolButton::sizeHint();
    size.rheight() += 20;
    size.rwidth() = qMax(size.width(), size.height());
    return size;
}

Button *GuiInterface::createButton(const QString &text, const char *member)
{
    Button *button = new Button(text);
    connect(button, SIGNAL(clicked()), this, member);
    return button;
}

QString GuiInterface::op() const
{
    return op_;
}

void GuiInterface::setOp(const QString &op)
{
    op_ = op;
}

double GuiInterface::rhs() const
{
    return rhs_;
}

void GuiInterface::setRhs(double rhs)
{
    rhs_ = rhs;
}

double GuiInterface::lhs() const
{
    return lhs_;
}

void GuiInterface::setLhs(double lhs)
{
    lhs_ = lhs;
}

GuiInterface::State GuiInterface::state() const
{
    return state_;
}

void GuiInterface::setState(const GuiInterface::State &state)
{
    state_ = state;
}

QString GuiInterface::result() const
{
    return result_;
}

void GuiInterface::setResult(const QString &result)
{
    if (result_ != result) {
        result_ = result;
        emit resultChanged(result_);
    }
}

GuiInterface::GuiInterface(QWidget *parent) : QWidget(parent),
    result_("0"),
    lhs_(0.0),
    rhs_(HUGE_VAL),
    op_(""),
    state_(WAITING_LHS)
{
    expression = new QLineEdit("");

    expression->setReadOnly(true);
    expression->setAlignment(Qt::AlignRight);
    //expression->setMaxLength(15);

    recentInput = new QLineEdit("0");

    recentInput->setReadOnly(true);
    recentInput->setAlignment(Qt::AlignRight);
    recentInput->setMaxLength(15);

    QFont font = recentInput->font();
    font.setPointSize(font.pointSize() + 8);
    recentInput->setFont(font);

    for (int i = 0; i < NumDigitButtons; ++i)
        digitButtons[i] = createButton(QString::number(i), SLOT(digitClicked()));

    Button *pointButton = createButton(tr("."), SLOT(pointClicked()));
    Button *changeSignButton = createButton(tr("\302\261"), SLOT(changeSignClicked()));

    Button *backspaceButton = createButton(tr("Backspace"), SLOT(backspaceClicked()));
    Button *clearButton = createButton(tr("Clear"), SLOT(clear()));
    Button *clearAllButton = createButton(tr("Clear All"), SLOT(clearAll()));

    Button *clearMemoryButton = createButton(tr("MC"), SLOT(clearMemory()));
    Button *readMemoryButton = createButton(tr("MR"), SLOT(readMemory()));
    Button *setMemoryButton = createButton(tr("MS"), SLOT(setMemory()));
    Button *addToMemoryButton = createButton(tr("M+"), SLOT(addToMemory()));

    Button *divisionButton = createButton(tr("\303\267"), SLOT(operatorClicked()));
    Button *timesButton = createButton(tr("\303\227"), SLOT(operatorClicked()));
    Button *minusButton = createButton(tr("-"), SLOT(operatorClicked()));
    Button *plusButton = createButton(tr("+"), SLOT(operatorClicked()));

    Button *squareRootButton = createButton(tr("\u221a"), SLOT(operatorClicked()));//0xE2 0x88 0x9A
    Button *powerButton = createButton(tr("x\302\262"), SLOT(operatorClicked()));
    Button *reciprocalButton = createButton(tr("1/x"), SLOT(operatorClicked()));

    Button *equalButton = createButton(tr("="), SLOT(equalClicked()));

    QGridLayout *mainLayout = new QGridLayout;

    mainLayout->setSizeConstraint(QLayout::SetFixedSize);

    mainLayout->addWidget(expression, 0, 0, 1, 6);
    mainLayout->addWidget(recentInput, 1, 0, 1, 6);

    mainLayout->addWidget(backspaceButton, 2, 0, 1, 2);
    mainLayout->addWidget(clearButton, 2, 2, 1, 2);
    mainLayout->addWidget(clearAllButton, 2, 4, 1, 2);

    mainLayout->addWidget(clearMemoryButton, 3, 0);
    mainLayout->addWidget(readMemoryButton, 4, 0);
    mainLayout->addWidget(setMemoryButton, 5, 0);
    mainLayout->addWidget(addToMemoryButton, 6, 0);

    for (int i = 1; i < NumDigitButtons; ++i) {
        int row = ((9 - i) / 3) + 3;
        int column = ((i - 1) % 3) + 1;
        mainLayout->addWidget(digitButtons[i], row, column);
    }

    mainLayout->addWidget(digitButtons[0], 6, 1);
    mainLayout->addWidget(pointButton, 6, 2);
    mainLayout->addWidget(changeSignButton, 6, 3);

    mainLayout->addWidget(divisionButton, 3, 4);
    mainLayout->addWidget(timesButton, 4, 4);
    mainLayout->addWidget(minusButton, 5, 4);
    mainLayout->addWidget(plusButton, 6, 4);

    mainLayout->addWidget(squareRootButton, 3, 5);
    mainLayout->addWidget(powerButton, 4, 5);
    mainLayout->addWidget(reciprocalButton, 5, 5);
    mainLayout->addWidget(equalButton, 6, 5);
    setLayout(mainLayout);

    setWindowTitle(tr("Calculator"));

    connect(this,  &GuiInterface::resultChanged, recentInput, &QLineEdit::setText);
}
//        NUM input
void GuiInterface::pointClicked(){
    auto r = result();
    if (!r.contains('.')) {
        setResult(r + '.');
        if (state() == WAITING_LHS) setState(LHS_INPUTTING);
        else if (state() == WAITING_RHS) setState(RHS_INPUTTING);
    }
}

void GuiInterface::changeSignClicked(){
    auto r = result();
    if (r != "0") {
        if (r.startsWith('-')) {
            setResult(r.remove(0, 1));
        } else {
            setResult(r.insert(0, '-'));
        }
    }
}

void GuiInterface::digitClicked(){
    Button *clickedButton = qobject_cast<Button *>(sender());
    auto digit = clickedButton->text();

    switch (state()) {
    case WAITING_LHS:
        if (digit != '0') {
            setResult(digit);
            setState(LHS_INPUTTING);
        }
        break;
    case WAITING_RHS:
        if (digit != '0') {
            setResult(digit);
            setState(RHS_INPUTTING);
        }
        break;
    case LHS_INPUTTING:
    case RHS_INPUTTING:
        setResult( result() + digit );
        break;
    default:
        break;
    }
}
//        Operator input
bool GuiInterface::checkOpPrecondition(const QString& op, double operand)//
{
    if ((op == tr("\u221a") && operand < 0.0)
        || (op == tr("1/x") && operand == 0.0)
        || (op == tr("\303\267") && operand == 0.0)) {
        return false;
    } else {
        return true;
    }

}

double GuiInterface::doUnaryCalculation(const QString& clickedOperator, double operand)
{
    double result = HUGE_VAL;

    if (clickedOperator == tr("\u221a")) {
        result = std::sqrt(operand);
    } else if (clickedOperator == tr("x\302\262")) {
        result = std::pow(operand, 2.0);
    } else if (clickedOperator == tr("1/x")) {
        result = 1.0 / operand;
    }

    setResult( QString::number( result ) );
    if (state() == LHS_INPUTTING) {
        setLhs( result );
    }
    else if (state() == RHS_INPUTTING) {
        setRhs( result );
    }

    return result;
}

double GuiInterface::doBinaryCalculation(double operand)
{
    double result = HUGE_VAL;
    QString o = op();
    setRhs(operand);

    if (o != "" && checkOpPrecondition(o, operand)) {
        if (o == tr("+")) {
            result = lhs() + rhs() ;
        } else if (o == tr("-")) {
            result = lhs() - rhs() ;
        } else if (o == tr("\303\227")) {
            result = lhs() * rhs() ;
        } else if (o == tr("\303\267")) {
            result = lhs() / rhs() ;
        }
        setLhs( result );
        setResult( QString::number( result ) );
    }

    return result;
}

bool GuiInterface::isUnaryOp(const QString& op) const
{
    return op == tr("\u221a") || op == tr("1/x") || op == tr("x\302\262");
}

bool GuiInterface::isBinaryOp(const QString& op) const
{
    return op == tr("\303\267") || op == "\303\227" || op == tr("-") || op == tr("+");
}

void GuiInterface::operatorClicked(){
    // operand complete, result->lhs or rhs conversion
    bool ok;
    auto conversion = result().toDouble(&ok);

    Button *clickedButton = qobject_cast<Button *>(sender());
    QString clickedOperator = clickedButton->text();

    if (isBinaryOp(clickedOperator)) {

        if (ok) {
            switch (state()) {
            case WAITING_LHS:
            case WAITING_RHS:
                break;
            case LHS_INPUTTING:
                setLhs(conversion);
                expression->setText( expression->text() + result() + clickedOperator);
                break;
            case RHS_INPUTTING:
                setRhs(conversion);
                expression->setText( expression->text() + result() + clickedOperator);
                doBinaryCalculation(conversion);
                break;
            default:
                break;
            }

            setOp(clickedOperator);
            setState(WAITING_RHS);
        }

    } else if (isUnaryOp(clickedOperator)) {

        doUnaryCalculation(clickedOperator, conversion);

    }
}

void GuiInterface::equalClicked(){

    if (state() == RHS_INPUTTING) {
        bool ok;
        auto conversion = result().toDouble(&ok);

        if (isBinaryOp( op() )) {
            doBinaryCalculation(conversion);

            setState(WAITING_LHS);

        } else {

        }
    } else if (state() == WAITING_LHS) {
        doBinaryCalculation(rhs());
    }

    expression->clear();
}

void GuiInterface::backspaceClicked(){
    auto r = result();
    auto bound = r.startsWith('-') ? 2 : 1;

    if ( bound < r.size()) {
        r.chop(1);
        setResult(r);
    }
    else {
        setResult("0");
        if (state()==LHS_INPUTTING) setState(WAITING_LHS);
        else if (state()==RHS_INPUTTING) setState(WAITING_RHS);
    }
}

void GuiInterface::clear(){
    expression->clear();
    setResult("0");
    switch (state()) {
    case WAITING_LHS:
    case WAITING_RHS:
        break;
    case LHS_INPUTTING:
        setLhs(0.0);
        setState(WAITING_LHS);
        break;
    case RHS_INPUTTING:
        setRhs(HUGE_VAL);
        setState(WAITING_RHS);
        break;
    default:
        break;
    }
}

void GuiInterface::clearAll(){
    expression->clear();
    setResult("0");
    setLhs(0.0);
    setRhs(HUGE_VAL);
    setOp("");
    setState(WAITING_LHS);
}

void GuiInterface::clearMemory(){}
void GuiInterface::readMemory(){}
void GuiInterface::setMemory(){}
void GuiInterface::addToMemory(){}
