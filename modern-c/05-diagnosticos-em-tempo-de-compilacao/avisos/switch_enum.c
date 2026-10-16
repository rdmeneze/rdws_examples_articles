enum estado { OCIOSO, MEDINDO, ERRO, CALIBRANDO };
const char *nome(enum estado e)
{
    switch (e) {
    case OCIOSO:  return "ocioso";
    case MEDINDO: return "medindo";
    case ERRO:    return "erro";
    default:      return "?";
    }
}
